#include <filesystem>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <stdio.h>

#include "image.hpp"

namespace fs = std::filesystem;

/* Returns true if PATH is a valid file path. */
static bool is_valid_file_path (const std::string &path);

/* Converts a valid file in FILE_PATH into a stringstream. */
static void convert_sstream (const std::string &file_path,
                             std::stringstream &);

/* Returns filename of a FILE_PATH . */
static std::string get_filename (std::string &file_path);

/* Returns 'uploadvideo' or 'uploadphoto'. */
static const std::string get_media_choice (MEDIA_TT &);

/* Returns 'overwrite', 'rename', or 'ignore'. */
static const std::string get_overwrite_choice (OVERWRITE_CHOICE &);

/* Adds KEY, VALUE pair in the form data. */
static const bool add_form_data (std::string key, std::string &path,
                                 httplib::MultipartFormDataItems &form_items,
                                 std::string filename);

/* Adds KEY, VALUE pair in the form data for auto generate thumb. */
static const bool
add_form_data_auto_thumb (std::string &path,
                          httplib::MultipartFormDataItems &form_items,
                          std::string filename);

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

static inline bool
is_base64 (BYTE c)
{
  return (isalnum (c) || (c == '+') || (c == '/'));
}

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
  std::string media_choice = get_media_choice (ctx.media);
  std::string overwrite_choice = get_overwrite_choice (ctx.ow);

  httplib::Client cli (http_ctx.domain, http_ctx.port);
  cli.set_read_timeout (60);
  httplib::MultipartFormDataItems items;

  if (ctx.filename.empty ())
    filename = get_filename (ctx.orig_path);
  else
    filename = ctx.filename;

  if (!add_form_data ("original", ctx.orig_path, items, filename))
    return false;

  if (ctx.auto_thumb)
    {
      // std::string heic_path = "D:\\images\\heic\\C003.heic";
      if (!add_form_data_auto_thumb (ctx.orig_path, items, filename))
        return false;
    }
  else
    {
      if (!add_form_data ("thumb_large", ctx.thumb_lg_path, items, filename))
        return false;
      if (!add_form_data ("thumb_small", ctx.thumb_sm_path, items, filename))
        return false;
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
  // std::string filename;
  // std::stringstream orig_ss;
  // auto orig_filename = convert_sstream (upload_ctx.orig_path, orig_ss);

  // if (!upload_ctx.filename.empty ())
  //   filename = upload_ctx.filename;
  // else
  //   filename = orig_filename;

  // std::string media_choice = get_media_choice (upload_ctx.media);
  // std::string overwrite_choice = get_overwrite_choice (upload_ctx.ow);

  // httplib::Client cli (http_ctx.domain, http_ctx.port);
  // httplib::MultipartFormDataItems items{
  //   { "original", orig_ss.str (), filename, "application/octet-stream" },
  // };

  // if (add_form_data ("high", add_ctx.high_res_path, items))
  //   return false;
  // if (add_form_data ("medium", add_ctx.medium_res_path, items))
  //   return false;
  // if (add_form_data ("low", add_ctx.low_res_path, items))
  //   return false;
  // if (add_form_data ("mobile", add_ctx.mobile_res_path, items))
  //   return false;
  // if (add_form_data ("iphone", add_ctx.iphone_res_path, items))
  //   return false;
  // if (add_form_data ("android", add_ctx.android_res_path, items))
  //   return false;
  // if (add_form_data ("flv", add_ctx.flv_res_path, items))
  //   return false;

  // httplib::Headers headers = {
  //   { "cookie", "PHPSESSID=" + http_ctx.token },
  // };

  // std::string query_string = "/photo/webapi/"
  //                            "file.php?api=SYNO.PhotoStation.File&version=1&"
  //                            "method="
  //                            + media_choice
  //                            + "&"
  //                              "dest_folder_path="
  //                            + upload_ctx.dest
  //                            + "&"
  //                              "duplicate="
  //                            + overwrite_choice
  //                            + "&"
  //                              "filename="
  //                            + filename
  //                            + "&"
  //                              "mtime=1579384308";

  // if (auto res = cli.Post (query_string, headers, items))
  //   {
  //     if (res->status == 200)
  //       {
  //         if (res->body.find ("true") != std::string::npos)
  //           return true;
  //         else
  //           return false;
  //       }

  //     else
  //       {
  //         auto err = res.error ();
  //         std::cerr << "Http error: " << httplib::to_string (err) <<
  //         std::endl; return false;
  //       }
  //   }
  // else
  //   {
  //     std::cerr << res->body << std::endl;
  //     return false;
  //   }

  return false;
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

static void
convert_sstream (const std::string &file_path, std::stringstream &ss)
{
  std::ifstream stream (file_path, std::ios::binary);
  ss << stream.rdbuf ();
}

static bool
is_valid_file_path (const std::string &path)
{
  fs::path file_path = path;

  if (fs::exists (file_path))
    return true;
  else
    {
      std::cout << path << " is not valid path." << std::endl;
      return false;
    }
}

static std::string
get_filename (std::string &file_path)
{
  fs::path path = file_path;
  std::string filename = path.filename ().string ();
  return filename;
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

static const bool
add_form_data (std::string key, std::string &path,
               httplib::MultipartFormDataItems &form_items,
               std::string filename)
{
  if (path.empty ())
    return true;

  if (!is_valid_file_path (path))
    return false;

  std::stringstream ss;
  convert_sstream (path, ss);

  form_items.push_back (
      { key, ss.str (), filename, "application/octet-stream" });

  return true;
}

static const bool
add_form_data_auto_thumb (std::string &path,
                          httplib::MultipartFormDataItems &form_items,
                          std::string filename)
{
  if (path.empty ())
    return true;

  if (!is_valid_file_path (path))
    return false;

  std::vector<uchar> buffer_s;
  std::vector<uchar> buffer_l;

  std::filesystem::path file_path = path;
  cv::Mat image;

  if (file_path.extension () == ".heic")
    {
      // std::string heic_path = "D:\\images\\heic\\C003.heic";
      std::string b64_str = SYNODER::get_heic_b64_data (path);
      std::vector<BYTE> decoded_str = SYNODER::base64_decode (b64_str);
      std::vector<char> im_vec (decoded_str.begin (), decoded_str.end ());
      image = cv::imdecode (im_vec, cv::IMREAD_COLOR);
    }
  else
    {
      image = cv::imread (path);
    }

  SYNODER::auto_generate_thumbnail (image, buffer_s, buffer_l);
  std::string binary_string_s (buffer_s.begin (), buffer_s.end ());
  std::string binary_string_l (buffer_l.begin (), buffer_l.end ());

  form_items.push_back ({ "thumb_small", std::move (binary_string_s), filename,
                          "application/octet-stream" });
  form_items.push_back ({ "thumb_large", std::move (binary_string_l), filename,
                          "application/octet-stream" });

  return true;
}

void
SYNODER::auto_generate_thumbnail (cv::Mat &image, std::vector<uchar> &buffer_s,
                                  std::vector<uchar> &buffer_l)
{
  std::cerr << "resizing!! " << std::endl;
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

  cv::imencode (".jpg", resized_img_s, buffer_s,
                std::vector<int>{ cv::IMWRITE_JPEG_QUALITY, 100 });
  cv::imencode (".jpg", resized_img_l, buffer_l,
                std::vector<int>{ cv::IMWRITE_JPEG_QUALITY, 100 });
}

std::string
SYNODER::get_heic_b64_data (std::string &path)
{
  char cmd[100];
  std::string command = "python convert_heic.py -p " + path;
  sprintf (cmd, command.c_str ());

  FILE *pipe;
#ifdef _WIN32
  pipe = _popen (cmd, "r");
#else
  pipe = popen (cmd, "r");
#endif

  if (!pipe)
    {
      std::cout << "Error: Failed to execute command." << std::endl;
      return "Error: Failed to execute command.";
    }

  char buffer[128];
  std::string result = "";
  while (!feof (pipe))
    {
      if (fgets (buffer, 128, pipe) != NULL)
        result += buffer;
    }

#ifdef _WIN32
  _pclose (pipe);
#else
  pclose (pipe);
#endif

  return result;
}

std::vector<BYTE>
SYNODER::base64_decode (std::string const &encoded_string)
{
  int in_len = encoded_string.size ();
  int i = 0;
  int j = 0;
  int in_ = 0;
  BYTE char_array_4[4], char_array_3[3];
  std::vector<BYTE> ret;

  while (in_len-- && (encoded_string[in_] != '=')
         && is_base64 (encoded_string[in_]))
    {
      char_array_4[i++] = encoded_string[in_];
      in_++;
      if (i == 4)
        {
          for (i = 0; i < 4; i++)
            char_array_4[i] = base64_chars.find (char_array_4[i]);

          char_array_3[0]
              = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
          char_array_3[1] = ((char_array_4[1] & 0xf) << 4)
                            + ((char_array_4[2] & 0x3c) >> 2);
          char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

          for (i = 0; (i < 3); i++)
            ret.push_back (char_array_3[i]);
          i = 0;
        }
    }

  if (i)
    {
      for (j = i; j < 4; j++)
        char_array_4[j] = 0;

      for (j = 0; j < 4; j++)
        char_array_4[j] = base64_chars.find (char_array_4[j]);

      char_array_3[0]
          = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1]
          = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (j = 0; (j < i - 1); j++)
        ret.push_back (char_array_3[j]);
    }

  return ret;
}
