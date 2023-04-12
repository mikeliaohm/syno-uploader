#include <args.hxx>
#include <conio.h>
#include <iostream>

#include "image.hpp"

using std::cout, std::endl;

int
main (int args, char *argv[])
{
  args::ArgumentParser parser ("This is a test program.",
                               "This goes after the options.");
  args::HelpFlag help (parser, "help", "Display this help menu",
                       { 'h', "help" });
  args::Group group (
      parser, "This group is all required:", args::Group::Validators::All);
  args::ValueFlag<std::string> domain (group, "domain", "The domain flag",
                                       { 'd', "domain" });
  args::ValueFlag<int> port (group, "port", "The port flag", { 'p', "port" });
  args::ValueFlag<std::string> dest (group, "dest", "The dest flag",
                                     { "dest" });
  args::ValueFlag<std::string> original (
      group, "original", "The original flag", { 'o', "original" });

  // TODO: --photo (default) --video
  // media_group choose only one or none
  args::Group media_group (parser, "This group need to choice a media type",
                           args::Group::Validators::AtMostOne);
  args::Flag photo (media_group, "photo", "The photo flag", { "photo" });
  args::Flag video (media_group, "video", "The video flag", { "video" });

  // TODO: --ignore (default) --overwrite -- rename
  // ow_group choose only one or none
  args::Group ow_group (parser,
                        "This group need to choice an overwrite action",
                        args::Group::Validators::AtMostOne);
  args::Flag ignore (ow_group, "ignore", "The ignore flag", { "ignore" });
  args::Flag overwrite (ow_group, "overwrite", "The overwrite flag",
                        { "overwrite" });
  args::Flag rename (ow_group, "rename", "The rename flag", { "rename" });

  // TODO: if no token, call SYNODER::authenticate
  args::ValueFlag<std::string> token (parser, "token", "The token flag",
                                      { 't', "token" });
  args::ValueFlag<std::string> thumb_small (
      parser, "thumb_small", "The thumb_small flag", { 's', "small" });
  args::ValueFlag<std::string> thumb_large (
      parser, "thumb_large", "The thumb_large flag", { 'l', "large" });
  args::ValueFlag<std::string> high (parser, "high", "The high flag",
                                     { "high" });
  args::ValueFlag<std::string> medium (parser, "medium", "The medium flag",
                                       { "medium" });
  args::ValueFlag<std::string> low (parser, "low", "The low flag",
                                    { "large" });
  args::ValueFlag<std::string> mobile (parser, "mobile", "The mobile flag",
                                       { "mobile" });
  args::ValueFlag<std::string> iphone (parser, "iphone", "The iphone flag",
                                       { "iphone" });
  args::ValueFlag<std::string> android (parser, "android", "The android flag",
                                        { "android" });
  args::ValueFlag<std::string> flv (parser, "flv", "The flv flag", { "flv" });
  args::ValueFlag<std::string> filename (parser, "filename",
                                         "The filename flag", { "filename" });
  args::ValueFlag<std::string> username (parser, "username",
                                         "The username flag", { "username" });
  args::ValueFlag<std::string> password (parser, "password",
                                         "The password flag", { "password" });
  args::Flag auto_thumb (parser, "auto_thumb", "The auto_thumb flag",
                         { "auto_thumb" });

  try
    {
      parser.ParseCLI (args, argv);
    }
  catch (args::Help)
    {
      cout << parser;
      return 0;
    }
  catch (args::ParseError e)
    {
      std::cerr << e.what () << endl;
      std::cerr << parser;
      return 1;
    }
  catch (args::ValidationError e)
    {
      std::cerr << e.what () << endl;
      std::cerr << parser;
      return 1;
    }

  MEDIA_TT media_tt;
  if (video)
    media_tt = MEDIA_TT::VIDEO;
  else
    media_tt = MEDIA_TT::PHOTO;

  OVERWRITE_CHOICE ow_choice;
  if (rename)
    ow_choice = OVERWRITE_CHOICE::RENAME;
  else if (overwrite)
    ow_choice = OVERWRITE_CHOICE::OVERWRITE;
  else
    ow_choice = OVERWRITE_CHOICE::IGNORE_DUPLICATE;

  UploadContext ctx{};
  ctx.orig_path = args::get (original);
  ctx.dest = args::get (dest);
  ctx.media = media_tt;
  ctx.ow = ow_choice;
  ctx.filename = args::get (filename);
  ctx.thumb_lg_path = args::get (thumb_large);
  ctx.thumb_sm_path = args::get (thumb_small);
  ctx.auto_thumb = args::get (auto_thumb);

  HttpContext http_ctx{};
  http_ctx.domain = args::get (domain);
  http_ctx.port = args::get (port);
  http_ctx.username = args::get (username);
  http_ctx.password = args::get (password);

  if (token)
    http_ctx.token = args::get (token);
  else
    {
      bool is_login = false;

      if (username && password)
        is_login = SYNODER::authenticate (http_ctx);

      while (!is_login)
        {
          std::string username, password;
          char c;
          cout << "Please enter your username" << endl;
          std::cin >> username;

          /* Display '*' symbols. */
          cout << "Please enter your password" << endl;
          while ((c = _getch ()) != '\r') // '\r' means the Enter key
            {
              if (c == '\b') // backspace
                {
                  if (!password.empty ())
                    {
                      password.pop_back ();
                      cout << "\b \b"; // erase last character on screen
                    }
                }
              else
                {
                  password.push_back (c);
                  cout << "*"; // display a star instead of the
                               // actual character
                }
            }

          http_ctx.username = username;
          http_ctx.password = password;

          is_login = SYNODER::authenticate (http_ctx);
        }
    }

  cout << "Uploading media files to Synology Photo Station." << endl;

  // TODO: init above the args::get and gather the fields one by one
  bool success;
  if (video)
    {
      AdditionalContext add_ctx{};
      add_ctx.high_res_path = args::get (high);
      add_ctx.medium_res_path = args::get (medium);
      add_ctx.low_res_path = args::get (low);
      add_ctx.mobile_res_path = args::get (mobile);
      add_ctx.iphone_res_path = args::get (iphone);
      add_ctx.android_res_path = args::get (android);
      add_ctx.flv_res_path = args::get (flv);
      success = SYNODER::upload_video (http_ctx, ctx, add_ctx);
    }
  else
    success = SYNODER::upload_image (http_ctx, ctx);

  if (success)
    {
      cout << "Upload succeeded." << endl;
    }
  else
    {
      cout << "Upload failed." << endl;
    }

  bool is_logout = SYNODER::logout (http_ctx);

  return 0;
}