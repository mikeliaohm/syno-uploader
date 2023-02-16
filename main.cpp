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
  std::string filename = "./test.png";
  std::ifstream istrm (filename, std::ios::binary);
  if (!istrm.is_open ())
    {
      std::cout << "Failed to open " << filename << std::endl;
      return EXIT_FAILURE;
    }

  // std::ifstream t_lf_img("C:\\Users\\lyyuli\\Pictures\\UploadTest.jpg");
  std::stringstream buffer_lf_img;
  buffer_lf_img << istrm.rdbuf();
  httplib::MultipartFormDataItems items 
    {
      { "api", "SYNO.PhotoStation.File", "", "" },
      { "version", "1", "", "" },
      { "method", "uploadphoto", "", "" },
      { "dest_folder_path", "dev/sub_folder", "", "" },
      { "duplicate", "ignore", "", "" },
      { "filename", "test1.jpg", "", "" },
      { "mtime", "1579384308", "", "" },
      { "original", buffer_lf_img.str(), "test1.jpg", "application/octet-stream" },
      { "thumb_small", buffer_lf_img.str(), "sm1.jpg", "application/octet-stream" },
      { "thumb_large", buffer_lf_img.str(), "lg1.jpg", "application/octet-stream" }
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