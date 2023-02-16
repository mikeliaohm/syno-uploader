#include <httplib.h>
#include <iostream>

using std::cout, std::endl;

int
main (int args, char *argv[])
{
  cout << "Uploading media files to Synology Photo Station." << endl;

  /* Sample http request using httplib. */
  /* login */
  httplib::Client cli ("114.43.204.232", 8009);
  // httplib::Params params 
  //   {
  //     { "api", "SYNO.PhotoStation.Auth" },
  //     { "version", "1" },
  //     { "method", "login" },
  //     { "username", "dev" },
  //     { "password", "ly29104335" }
  //   };

  // if (auto res = cli.Post ("/photo/webapi/auth.php", params))
  //   {
  //     if (res->status == 200)
  //       cout << res->body << endl;
  //   }
  // else
  //   {
  //     auto err = res.error ();
  //     cout << "Http error: " << httplib::to_string (err) << endl;
  //   }

  /* upload */
  std::ifstream t_lf_img("C:\\Users\\lyyuli\\Pictures\\UploadTest.jpg");
  std::stringstream buffer_lf_img;
  buffer_lf_img << t_lf_img.rdbuf();
  httplib::MultipartFormDataItems items 
    {
      { "api", "SYNO.PhotoStation.File", "", "" },
      { "version", "1", "", "" },
      { "method", "uploadphoto", "", "" },
      { "dest_folder_path", "dev/sub_folder", "", "" },
      { "duplicate", "rename", "", "" },
      { "filename", "UploadTest.jpg", "", "" },
      { "mtime", "1579384308", "", "" },
      { "original", buffer_lf_img.str(), "UploadTest.jpg", "image/jpeg" }
    };
  httplib::Headers headers = 
    {
      { "cookie", "PHPSESSID=d3c8542d8e668c19adb14523d328ce44" },
      // { "content-type", "multipart/form-data" },
      // { "x-syno-token", "d3c8542d8e668c19adb14523d328ce44" }
    };

  if (auto res = cli.Post ("/photo/webapi/file.php", headers, items))
    {
      if (res->status == 200)
        cout << res->body << endl;
    }
  else
    {
      auto err = res.error ();
      cout << "Http error: " << httplib::to_string (err) << endl;
    }

  return 0;
}