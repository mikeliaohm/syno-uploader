#include <filesystem>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>

namespace fs = std::filesystem;

#include "image.hpp"

/* Returns true if PATH is a valid file path. */
static bool is_valid_file_path (const std::string &path);

/* Converts a valid file in FILE_PATH into a stringstream.
   Returns the filename if FILE_PATH is valid. */
static const std::string convert_sstream (const std::string &file_path,
                                          std::stringstream &);

/* Returns 'uploadvideo' or 'uploadphoto'. */
static const std::string get_media_choice (MEDIA_TT &);

/* Returns 'overwrite', 'rename', or 'ignore'. */
static const std::string get_overwrite_choice (OVERWRITE_CHOICE &);

/* Adds KEY, VALUE pair in the form data. */
static const void add_form_data (std::string key, std::string &value,
                                 httplib::MultipartFormDataItems &);

static const void auto_generate_thumbnail (std::string &path,
                                           httplib::MultipartFormDataItems &,
                                           std::string &filename);

bool
SYNODER::authenticate (struct HttpContext &http_ctx)
{
  httplib::Client cli (http_ctx.domain, http_ctx.port);

  /* login */
  httplib::Params params{ { "api", "SYNO.PhotoStation.Auth" },
                          { "version", "1" },
                          { "method", "login" },
                          { "username", http_ctx.username },
                          { "password", http_ctx.password } };

  if (auto res = cli.Post ("/photo/webapi/auth.php", params))
    {
      if (res->status == 200)
        {
          std::regex regex_pattern ("\"sid\":\"([^\"]+)\"");
          std::smatch regex_match;
          if (std::regex_search (res->body, regex_match, regex_pattern))
            {
              std::string sid = regex_match[1].str ();
              http_ctx.token = sid;
              std::cout << "Login succeeded." << std::endl;
              return true;
            }
          else
            {
              std::cerr << "Cannot retrieve login token." << std::endl;
              return false;
            }
        }
      else
        {
          auto err = res.error ();
          std::cerr << "Http error: " << httplib::to_string (err) << std::endl;
          return false;
        }
    }
  else
    {
      auto err = res.error ();
      std::cerr << "Http error: " << httplib::to_string (err) << std::endl;
      return false;
    }
}

bool
SYNODER::upload_image (struct HttpContext &http_ctx, struct UploadContext &ctx)
{
  std::string filename;
  std::stringstream orig_ss;
  auto orig_filename = convert_sstream (ctx.orig_path, orig_ss);

  if (!ctx.filename.empty ())
    filename = ctx.filename;
  else
    filename = orig_filename;

  std::string media_choice = get_media_choice (ctx.media);
  std::string overwrite_choice = get_overwrite_choice (ctx.ow);

  httplib::Client cli (http_ctx.domain, http_ctx.port);
  httplib::MultipartFormDataItems items{
    { "original", orig_ss.str (), filename, "application/octet-stream" },
  };

  if (ctx.auto_thumb)
    {
      auto_generate_thumbnail (ctx.orig_path, items, filename);
    }
  else
    {
      add_form_data ("thumb_large", ctx.thumb_lg_path, items);
      add_form_data ("thumb_small", ctx.thumb_sm_path, items);
    }

  httplib::Headers headers = {
    { "cookie", "PHPSESSID=" + http_ctx.token },
  };

  std::string query_string = "/photo/webapi/"
                             "file.php?api=SYNO.PhotoStation.File&version=1&"
                             "method="
                             + media_choice
                             + "&"
                               "dest_folder_path="
                             + ctx.dest
                             + "&"
                               "duplicate="
                             + overwrite_choice
                             + "&"
                               "filename="
                             + filename
                             + "&"
                               "mtime=1579384308";

  if (auto res = cli.Post (query_string, headers, items))
    {
      if (res->status == 200)
        {
          std::cout << res->body << std::endl;
          if (res->body.find ("true") != std::string::npos)
            return true;
          else
            return false;
        }

      else
        {
          auto err = res.error ();
          std::cerr << "Http error: " << httplib::to_string (err) << std::endl;
          return false;
        }
    }
  else
    {
      std::cerr << res->body << std::endl;
      return false;
    }
}

bool
SYNODER::upload_video (struct HttpContext &http_ctx,
                       struct UploadContext &upload_ctx,
                       struct AdditionalContext &add_ctx)
{
  std::string filename;
  std::stringstream orig_ss;
  auto orig_filename = convert_sstream (upload_ctx.orig_path, orig_ss);

  if (!upload_ctx.filename.empty ())
    filename = upload_ctx.filename;
  else
    filename = orig_filename;

  std::string media_choice = get_media_choice (upload_ctx.media);
  std::string overwrite_choice = get_overwrite_choice (upload_ctx.ow);

  httplib::Client cli (http_ctx.domain, http_ctx.port);
  httplib::MultipartFormDataItems items{
    { "original", orig_ss.str (), filename, "application/octet-stream" },
  };

  add_form_data ("high", add_ctx.high_res_path, items);
  add_form_data ("medium", add_ctx.medium_res_path, items);
  add_form_data ("low", add_ctx.low_res_path, items);
  add_form_data ("mobile", add_ctx.mobile_res_path, items);
  add_form_data ("iphone", add_ctx.iphone_res_path, items);
  add_form_data ("android", add_ctx.android_res_path, items);
  add_form_data ("flv", add_ctx.flv_res_path, items);

  httplib::Headers headers = {
    { "cookie", "PHPSESSID=" + http_ctx.token },
  };

  std::string query_string = "/photo/webapi/"
                             "file.php?api=SYNO.PhotoStation.File&version=1&"
                             "method="
                             + media_choice
                             + "&"
                               "dest_folder_path="
                             + upload_ctx.dest
                             + "&"
                               "duplicate="
                             + overwrite_choice
                             + "&"
                               "filename="
                             + filename
                             + "&"
                               "mtime=1579384308";

  if (auto res = cli.Post (query_string, headers, items))
    {
      if (res->status == 200)
        {
          if (res->body.find ("true") != std::string::npos)
            return true;
          else
            return false;
        }

      else
        {
          auto err = res.error ();
          std::cerr << "Http error: " << httplib::to_string (err) << std::endl;
          return false;
        }
    }
  else
    {
      std::cerr << res->body << std::endl;
      return false;
    }
}

bool
SYNODER::logout (struct HttpContext &http_ctx)
{
  httplib::Client cli (http_ctx.domain, http_ctx.port);
  httplib::Params params{ { "api", "SYNO.PhotoStation.Auth" },
                          { "version", "1" },
                          { "method", "logout" } };
  httplib::Headers headers = {
    { "cookie", "PHPSESSID=" + http_ctx.token },
  };

  if (auto res = cli.Post ("/photo/webapi/auth.php", headers, params))
    {
      if (res->status == 200)
        {
          if (res->body.find ("true") != std::string::npos)
            {
              std::cout << "Logout succeeded." << std::endl;
              return true;
            }
          else
            return false;
        }
      else
        {
          auto err = res.error ();
          std::cout << "Http error: " << httplib::to_string (err) << std::endl;
          return false;
        }
    }
  else
    {
      auto err = res.error ();
      std::cout << "Http error: " << httplib::to_string (err) << std::endl;
      return false;
    }
}

static const std::string
convert_sstream (const std::string &file_path, std::stringstream &ss)
{
  // call filesystem library in cpp std library

  if (!is_valid_file_path (file_path))
    {
      std::cerr << file_path << " is not valid path." << std::endl;
      return {};
    }
  else
    {
      std::ifstream stream (file_path, std::ios::binary);
      ss << stream.rdbuf ();

      fs::path path = file_path;
      std::string filename = path.filename ().string ();
      return filename;
    }
}

static bool
is_valid_file_path (const std::string &path)
{
  fs::path file_path = path;

  if (fs::exists (file_path))
    return true;
  else
    return false;
}

static const std::string
get_media_choice (MEDIA_TT &tt)
{
  std::string media_choice;

  switch (tt)
    {
    case MEDIA_TT::VIDEO:
      media_choice = "uploadvideo";
      break;
    default:
      media_choice = "uploadphoto";
      break;
    }

  return media_choice;
}

static const std::string
get_overwrite_choice (OVERWRITE_CHOICE &choice)
{
  std::string overwrite_choice;

  switch (choice)
    {
    case OVERWRITE_CHOICE::OVERWRITE:
      overwrite_choice = "overwrite";
      break;
    case OVERWRITE_CHOICE::RENAME:
      overwrite_choice = "rename";
      break;
    default:
      overwrite_choice = "ignore";
      break;
    }

  return overwrite_choice;
}

static const void
add_form_data (std::string key, std::string &value,
               httplib::MultipartFormDataItems &form_items)
{
  if (!value.empty ())
    {
      std::stringstream ss;
      auto filename = convert_sstream (value, ss);
      form_items.push_back (
          { key, ss.str (), filename, "application/octet-stream" });
    }
}

static const void
auto_generate_thumbnail (std::string &path,
                         httplib::MultipartFormDataItems &form_items,
                         std::string &filename)
{
  std::cerr << "resizing!! " << std::endl;
  cv::Mat image = cv::imread (path);
  int height = image.rows;
  int width = image.cols;
  int height_s, width_s, height_l, width_l;

  if (height <= 320 || width <= 320)
    {
      width_s = width;
      height_s = height;
      width_l = width;
      height_l = height;
    }
  else if ((height > 320 && height <= 1280) || (width > 320 && width <= 1280))
    {
      width_l = width;
      height_l = height;

      if (height > width)
        {
          width_s = 320;
          height_s = 320 * height / width;
        }
      else
        {
          height_s = 320;
          width_s = 320 * width / height;
        }
    }
  else
    {
      if (height > width)
        {
          width_s = 320;
          height_s = 320 * height / width;
          width_l = 1280;
          height_l = 1280 * height / width;
        }
      else
        {
          height_s = 320;
          width_s = 320 * width / height;
          height_l = 1280;
          width_l = 1280 * width / height;
        }
    }

  cv::Mat resized_img_s;
  cv::resize (image, resized_img_s, cv::Size (width_s, height_s));
  cv::Mat resized_img_l;
  cv::resize (image, resized_img_l, cv::Size (width_l, height_l));

  std::vector<uchar> buffer_s;
  cv::imencode (".jpg", resized_img_s, buffer_s,
                std::vector<int>{ cv::IMWRITE_JPEG_QUALITY, 100 });
  std::string binary_string_s (buffer_s.begin (), buffer_s.end ());
  std::vector<uchar> buffer_l;
  cv::imencode (".jpg", resized_img_l, buffer_l,
                std::vector<int>{ cv::IMWRITE_JPEG_QUALITY, 100 });
  std::string binary_string_l (buffer_l.begin (), buffer_l.end ());

  // cv::imwrite("small.jpg", resized_img_s); // create small.jpg
  // cv::imwrite("large.jpg", resized_img_l); // create large.jpg

  form_items.push_back (
      { "thumb_small", binary_string_s, filename, "application/octet-stream" });
  form_items.push_back (
      { "thumb_large", binary_string_l, filename, "application/octet-stream" });
}
