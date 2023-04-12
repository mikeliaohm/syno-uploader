// Copyright (c) 2023 Mike Liao
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <string>

enum MEDIA_TT
{
  PHOTO,
  VIDEO
};

enum OVERWRITE_CHOICE
{
  IGNORE_DUPLICATE,
  OVERWRITE,
  RENAME
};

struct HttpContext
{
  std::string domain;
  int port;
  std::string token;
  std::string username;
  std::string password;
};

struct UploadContext
{
  MEDIA_TT media;
  std::string dest;
  OVERWRITE_CHOICE ow;
  std::string orig_path;
  std::string thumb_sm_path;
  std::string thumb_lg_path;
  std::string filename;
  bool auto_thumb;
};

struct AdditionalContext
{
  std::string high_res_path;
  std::string medium_res_path;
  std::string low_res_path;
  std::string mobile_res_path;
  std::string iphone_res_path;
  std::string android_res_path;
  std::string flv_res_path;
};

struct InMemoryUploadContext
{
};

namespace SYNODER
{
/* Returns true if login credential is verified. */
bool authenticate (struct HttpContext &http_ctx);

/* Uploads image files and thumbnails instructed in CTX. */
bool upload_image (struct HttpContext &http_ctx, struct UploadContext &ctx);

/* Uploads video files and thumbnails instructed in CTX and ADD_CTX.  */
bool upload_video (struct HttpContext &http_ctx, struct UploadContext &ctx,
                   struct AdditionalContext &add_ctx);

/* Returns true if logout succeeds. */
bool logout (struct HttpContext &http_ctx);
}