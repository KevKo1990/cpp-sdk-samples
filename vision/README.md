# Sample apps for analyzing facial emotion using Affectiva's Automotive SDK for Linux

###frame-detector-webcam-demo

This sample demonstrates use of the [FrameDetector class](https://auto.affectiva.com/docs/vision-create-detector), getting its input from a webcam. It analyzes received frames and displays the results on screen.

After building, run the command `./frame-detector-webcam-demo --help` for information on its command line options.

###frame-detector-video-demo

This sample demonstrates use of the [FrameDetector class](https://auto.affectiva.com/docs/vision-create-detector), getting its input from a video file. It analyzes received frames and displays the results on screen.

After building, run the command `./frame-detector-video-demo --help` for information on its command line options.

---
Note: Both of these sample apps take a command line parameter (`-d/--data`) which is used to specify the path to the data directory for the Affectiva Vision library.  This directory is named `data/vision`, and is located under the folder where you installed the Affectiva Automotive SDK (e.g. `/path/to/affectiva-sdk/vision/data`).

---

## Dependencies

#### Affectiva Vision library

The Vision Library is packaged with the Automotive SDK, which is available upon request. To get access, please [contact us](https://auto.affectiva.com/).

#### Boost 1.63

See http://www.boost.org/

```
$ mkdir boost-build
$ cd boost-build
$ wget https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.gz
$ tar -xzvf boost_1_63_0.tar.gz
$ cd boost_1_63_0
$ ./bootstrap.sh
$ sudo ./b2 -j $(nproc) cxxflags=-fPIC threading=multi runtime-link=shared \
      --with-filesystem --with-program_options install
```

#### OpenCV and CMake

Ubuntu:
`$ sudo apt-get install -y build-essential libopencv-dev cmake`

## Building with CMake

Specify the the following CMake variables to identify the locations of various dependencies:

- **AFFECTIVA_SDK_DIR**: path to the folder where the Automotive SDK is installed
- **BOOST_ROOT** path to the Boost src tree


#### Linux (x86_64, aarch64)

For building under Linux, type the following command:

`$ cmake . -DCMAKE_BUILD_TYPE=[Release,Debug] <other args>`

Example script (replace directories starting with `/path/to` as appropriate):
```
# create a build directory
mkdir vision-samples-build/
cd vision-samples-build

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release \
-DAFFECTIVA_SDK_DIR=/path/to/auto-sdk \
-DBOOST_ROOT=/path/to/boost-build \
-DOpenCV_DIR=/path/to/opencv \
-DCMAKE_INSTALL_PREFIX=/path/to/install"

# run cmake, specifying the path to the CMakeLists.txt file (the same directory
# that this README.md file resides in)
cmake $CMAKE_ARGS /path/to/cpp-sdk-samples/vision
make -j4
make install

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AFFECTIVA_SDK_DIR/lib
```

The Affectiva SDK statically links a customized version of OpenCV, so if you run into double free or corruption errors, then you will need to preload the OpenCV library installed from package manager.

Use this command to find the path of libopencv_core.so.2.4

`ldconfig -p | grep libopencv_core.so.2.4`

Then set LD_PRELOAD to ensure it gets loaded first at runtime:
`export LD_PRELOAD=/path/to/libopencv_core.so.2.4`

##Docker Build Instructions

A Dockerfile is located in the top-level directory of this repo ([here](../Dockerfile)). To build the docker image, please refer to that file for instructions.
