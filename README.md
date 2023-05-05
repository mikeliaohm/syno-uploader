# SYNO-UPLOADER

## Introduction

This repo implements a C++ library to upload images and videos to Synology's Photo Station. Photo Station is an app that provides web interfaces for users to manage their photos and videos in Synology's various NAS products. This library is tested in Photo Station 6 (version 6.8.16-3506). Photo Station exposes several web APIs for accessing and editing images and videos stored in the NAS. Although there is no official documentation for the APIs, you can still find some information. For example, [jbowen.dev](https://blog.jbowen.dev/2020/01/exploring-the-synology-photostation-api/) provides detailed information about some of the APIs and their uses. Also, since these APIs are written in php, it should be relatively easy to look into the source code and figure out what they are about.

## Features

- Authenticate.
- Upload Photo.
- Upload Video. (Still under test)

## Installation and Dependencies

### httplib and args:

The repo makes use of [cpp-httplib](https://github.com/yhirose/cpp-httplib) for the http requests. It is added as a  git submodule in the `external/httplib` folder. To download the source code of the cpp-httplib library. Use the following command. The sample executable has another dependency [args parser github](https://github.com/Taywee/args.git) to parse invoking arguments of the executable.

```bash
# add the packages into the project
git submodule update --init --recursive
```

### opencv

Download opencv 4.7.0 [link](https://opencv.org/releases/)

- **Windows:**

1. Upzip to C, the path need to be `C:\opencv`, then CMakeList.txt can find it
2. Add value `C:\opencv\build\x64\vc16\bin` to PATH environment variables

- **Mac:**

1. Upzip to /usr/local, the path need to be `/usr/local/opencv/build`, than CMakeList.txt can find it
2. complie opencv, follow steps below or refer to [docs](https://docs.opencv.org/3.4/d0/db2/tutorial_macos_install.html)
- `cd /usr/local/opencv && mkdir build && cd build`
- `cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..`
- `make -j7`

Afterwards, build the project by creating a build directory and build it. For example,

## Build and Test

```bash
# In the source root
mkdir build && cd build

cmake ..
cmake --build .

# Run the executable
./Debug/SYNODER.exe     # in Windows
./SYNODER               # in MacOS
```

## Sample Uses

To use the libray, include the header `image.hpp`, which contains separate APIs for uploading images and videos. The API will upload one file (along with all its thumbnails) at a time. For example, to upload a photo in the build directory, try the following sample script.

```bash
  [path_to_build]/SYNODER \
  -d [NAS ip] \             # -d to specify the NAS IP address.
  -p [NAS port] \           # -p to specify the NAS port.
  --original photo.png \    # File path of the image.
  --small sm_photo.png \    # File path of the small size thumbnail.
  --large lg_photo.png \    # File path of the large size thumbnail.
  --dest [NAS folder] \     # NAS folder identifies the destination folder in 
                            # Photo Station
  --photo \                 # Upload the photo.
  --rename \
  --auto_thumb              # auto generate thumbnail.
```

## Known issues

The uploading API in Photo Station provides a configuration to deal with the situation when a file to be uploaded has a conflicting filename. There are three choices - **overwrite**, **ignore**, and **rename**. These choices should be self explanatory. However, there is probably a bug in the **rename** choice. When a filename conflicts, the code will append the filename of the photo to be uploaded with increments such as `_1`, `_2`, and etc. However, the thumbnails contained in the same uploading http request do not have their names appended with those same increments. The code will then store these new thumbnails in the NAS filesystem and overwrite the thumbnails associated with the existing image/video. The bug is a bit subtle but if you have access to the php source code, trace carefully through the logic of the renaming part and you should be able to spot it.
