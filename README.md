# SYNO-UPLOADER

## Installation

The repo makes use of [cpp-httplib](https://github.com/yhirose/cpp-httplib) for the http requests. It is added as a  git submodule in the `external/httplib` folder. To download the source code of the cpp-httplib library. Use the following command.

```bash
git submodule update --init --recursive
```

Afterwards, build the project by creating a build directory and build it. For example,

```bash
# In the source root
mkdir build && cd build

cmake ..
cmake --build .

# Run the executable
./Debug/SYNODER.exe     # in Windows
./SYNODER               # in MacOS
```
