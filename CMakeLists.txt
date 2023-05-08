cmake_minimum_required(VERSION 3.10)

# set the project name
project(SYNODER VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(LIB_SYNODER libsynoder)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(WIN32)
    set(OpenCV_DIR "C:/opencv/build")
else()
    set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk") 
    set(OpenCV_DIR "/usr/local/opencv/build")
endif()

add_subdirectory(external/httplib)
add_subdirectory(external/args)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# add_custom_target(
#   opencv_build
#   COMMAND cd /usr/local/opencv && ${CMAKE_COMMAND} -E make_directory build
#   COMMAND cd /usr/local/opencv/build && ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
#   COMMAND cd /usr/local/opencv/build && ${CMAKE_COMMAND} --build . -- -j7
# )

include_directories(include)
add_library(${LIB_SYNODER} src/image.cpp include/image.hpp)

# add the executable
add_executable(${CMAKE_PROJECT_NAME} main.cpp)
add_executable(OPENCV_CLIENT opencv_client.cpp)

target_link_libraries(${LIB_SYNODER} PUBLIC 
  httplib::httplib
  ${OpenCV_LIBS}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
  ${LIB_SYNODER}
  args
)

target_link_libraries(OPENCV_CLIENT PRIVATE 
  ${LIB_SYNODER}
)
