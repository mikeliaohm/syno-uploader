cmake_minimum_required(VERSION 3.10)

# set the project name
project(SYNODER VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(LIB_SYNODER libsynoder)

add_subdirectory(external/httplib)
add_subdirectory(external/args)

include_directories(include)
add_library(${LIB_SYNODER} src/image.cpp)

# add the executable
add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_link_libraries(${LIB_SYNODER} PUBLIC 
  httplib::httplib
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
  ${LIB_SYNODER}
  args
)
