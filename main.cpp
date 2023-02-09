#include <httplib.h>
#include <iostream>

using std::cout, std::endl;

int
main (int args, char *argv[])
{
  cout << "Uploading media files to Synology Photo Station." << endl;

  /* Sample http request using httplib. */
  httplib::Client cli ("www.google.com", 80);
  if (auto res = cli.Get ("/"))
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