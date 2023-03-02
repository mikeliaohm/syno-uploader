#include <filesystem>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

#include "image.hpp"

static bool is_valid_file_path (const std::string &path);

const std::string convert_sstream (const std::string &file_path,
                                   std::stringstream &);

const std::string get_media_choice (MEDIA_TT &);
const std::string get_overwrite_choice (OVERWRITE_CHOICE &);
const void update_addtional_item (std::string key, std::string &path,
                                  httplib::MultipartFormDataItems &items);

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
              std::cout << "login successed!!" << std::endl;
              return true;
            }
          else
            {
              std::cout << "No match found." << std::endl;
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
  else
    {
      auto err = res.error ();
      std::cout << "Http error: " << httplib::to_string (err) << std::endl;
      return false;
    }
}

bool
SYNODER::upload_image (struct HttpContext &http_ctx, struct UploadContext &ctx)
{
  std::stringstream orig_ss;
  auto orig_filename = convert_sstream (ctx.orig_path, orig_ss);

  std::string media_choice = get_media_choice (ctx.media);
  std::string overwrite_choice = get_overwrite_choice (ctx.ow);

  httplib::Client cli (http_ctx.domain, http_ctx.port);
  httplib::MultipartFormDataItems items{
    { "original", orig_ss.str (), orig_filename, "application/octet-stream" },
  };

  update_addtional_item ("thumb_large", ctx.thumb_lg_path, items);
  update_addtional_item ("thumb_small", ctx.thumb_sm_path, items);

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
                             + orig_filename
                             + "&"
                               "mtime=1579384308";

  if (auto res = cli.Post (query_string, headers, items))
    {
      if (res->status == 200)
        {
          std::cout << res->body << std::endl;
          return true;
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
      std::cout << res->body << std::endl;
      return false;
    }
}

bool
SYNODER::upload_video (struct HttpContext &http_ctx,
                       struct UploadContext &upload_ctx,
                       struct AdditionalContext &add_ctx)
{
  std::stringstream orig_ss;
  auto orig_filename = convert_sstream (upload_ctx.orig_path, orig_ss);

  std::string media_choice = get_media_choice (upload_ctx.media);
  std::string overwrite_choice = get_overwrite_choice (upload_ctx.ow);

  httplib::Client cli (http_ctx.domain, http_ctx.port);
  httplib::MultipartFormDataItems items{
    { "original", orig_ss.str (), orig_filename, "application/octet-stream" },
  };

  update_addtional_item ("high", add_ctx.high_res_path, items);
  update_addtional_item ("medium", add_ctx.medium_res_path, items);
  update_addtional_item ("low", add_ctx.low_res_path, items);
  update_addtional_item ("mobile", add_ctx.mobile_res_path, items);
  update_addtional_item ("iphone", add_ctx.iphone_res_path, items);
  update_addtional_item ("android", add_ctx.android_res_path, items);
  update_addtional_item ("flv", add_ctx.flv_res_path, items);

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
                             + orig_filename
                             + "&"
                               "mtime=1579384308";

  if (auto res = cli.Post (query_string, headers, items))
    {
      if (res->status == 200)
        {
          std::cout << res->body << std::endl;
          return true;
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
      std::cout << res->body << std::endl;
      return false;
    }
}

bool
SYNODER::logout (struct HttpContext &http_ctx)
{
  httplib::Client cli (http_ctx.domain, http_ctx.port);

  /* login */
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
          std::cout << "logout successed!!" << std::endl;
          return true;
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

const std::string
convert_sstream (const std::string &file_path, std::stringstream &ss)
{
  // call filesystem library in cpp std library

  if (!is_valid_file_path (file_path))
    {
      std::cout << file_path << "not valid path" << std::endl;
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

const std::string
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

const std::string
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

const void
update_addtional_item (std::string key, std::string &path,
                       httplib::MultipartFormDataItems &items)
{
  if (!path.empty ())
    {
      std::stringstream ss;
      auto filename = convert_sstream (path, ss);
      items.push_back (
          { key, ss.str (), filename, "application/octet-stream" });
    }
}
