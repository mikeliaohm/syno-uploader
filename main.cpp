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
  std::string thumbname = "./green.png";
  std::string thumbname_red = "./red.jpg";
  std::ifstream istrm (filename, std::ios::binary);
  std::ifstream thumbstrm (thumbname, std::ios::binary);
  std::ifstream thumbstrm_red (thumbname_red, std::ios::binary);

  if (!istrm.is_open ())
    {
      std::cout << "Failed to open " << filename << std::endl;
      return EXIT_FAILURE;
    }

  if (!thumbstrm.is_open ())
    {
      std::cout << "Failed to open " << thumbname << std::endl;
      return EXIT_FAILURE;
    }

  if (!thumbstrm_red.is_open ())
    {
      std::cout << "Failed to open " << thumbname_red << std::endl;
      return EXIT_FAILURE;
    }

  // std::ifstream t_lf_img("C:\\Users\\lyyuli\\Pictures\\UploadTest.jpg");
  std::stringstream buffer_lf_img;
  std::stringstream thumb_lg;
  std::stringstream thumb_sm;
  buffer_lf_img << istrm.rdbuf();
  thumb_lg << thumbstrm.rdbuf();
  thumb_sm << thumbstrm_red.rdbuf();
  httplib::MultipartFormDataItems items 
    {
      { "original", buffer_lf_img.str(), "test2.jpg", "application/octet-stream" },
      { "thumb_large", thumb_lg.str(), "THUMB_XL.jpg", "image/png" },
      // { "thumb_large", thumb_lg.str(), "THUMB_L.jpg", "image/png" },
      // { "thumb_small", thumb_lg.str(), "THUMB_M.jpg", "image/png" },
      { "thumb_small", thumb_sm.str(), "THUMB_M.jpg", "image/jpg" },
    };
  httplib::Headers headers = 
    {
      { "cookie", "PHPSESSID=d3c8542d8e668c19adb14523d328ce44" },
      // { "content-type", "multipart/form-data" },
      // { "x-syno-token", "d3c8542d8e668c19adb14523d328ce44" }
    };

  if (auto res = cli.Post ("/photo/webapi/file.php?api=SYNO.PhotoStation.File&method=uploadphoto&version=1&dest_folder_path=dev/sub_folder&duplicate=ignore&filename=myfilename1.jpg&mtime=1579384308", headers, items))
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