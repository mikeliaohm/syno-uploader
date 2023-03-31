#include <filesystem>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

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
static const bool add_form_data (std::string key, std::string &value,
                                 httplib::MultipartFormDataItems &);

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

  if (!add_form_data ("thumb_large", ctx.thumb_lg_path, items))
    return false;
  if (!add_form_data ("thumb_small", ctx.thumb_sm_path, items))
    return false;

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

  if (add_form_data ("high", add_ctx.high_res_path, items))
    return false;
  if (add_form_data ("medium", add_ctx.medium_res_path, items))
    return false;
  if (add_form_data ("low", add_ctx.low_res_path, items))
    return false;
  if (add_form_data ("mobile", add_ctx.mobile_res_path, items))
    return false;
  if (add_form_data ("iphone", add_ctx.iphone_res_path, items))
    return false;
  if (add_form_data ("android", add_ctx.android_res_path, items))
    return false;
  if (add_form_data ("flv", add_ctx.flv_res_path, items))
    return false;

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

  std::ifstream stream (file_path, std::ios::binary);
  ss << stream.rdbuf ();

  fs::path path = file_path;
  std::string filename = path.filename ().string ();
  return filename;
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

static const bool
add_form_data (std::string key, std::string &value,
               httplib::MultipartFormDataItems &form_items)
{
  if (!value.empty ())
    {
      std::stringstream ss;
      if (is_valid_file_path (value))
        {
          auto filename = convert_sstream (value, ss);
          form_items.push_back (
              { key, ss.str (), filename, "application/octet-stream" });

          return true;
        }
      else {
        std::cerr << value << " is not valid path." << std::endl;
        return false;
      }
    }
  else
    return true;
}

std::string
SYNODER::read_password ()
{
  std::string password = "";
  char c;
  bool visible;

  /* Display '*' symbols. */
  std::cout << "Please enter your password" << std::endl;

#ifdef _WIN32
  visible = false;
  while ((c = _getch ()) != '\r') // '\r' means the Enter key
    {
      if (c == '\b') // backspace
        {
          if (!password.empty ())
            {
              password.pop_back ();
              std::cout << "\b \b"; // erase last character on screen
            }
        }
      else
        {
          password.push_back (c);
          std::cout << "*"; // display a star instead of the
                            // actual character
        }
    }
#else
  visible = false;

  termios oldt, newt;
  tcgetattr (STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr (STDIN_FILENO, TCSANOW, &newt);

  while ((c = getchar ()) != '\n')
    {
      if (c == 127 && password.size () > 0)
        {
          password.pop_back ();
          std::cout << "\b \b";
        }
      else if (isprint (c) && visible)
        {
          password += c;
          std::cout << "*";
        }
    }

  tcsetattr (STDIN_FILENO, TCSANOW, &oldt);
#endif

  return password;
}
