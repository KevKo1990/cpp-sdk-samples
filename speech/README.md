# Sample apps for analyzing speaker emotion using Affectiva's Automotive SDK for Linux

###mic

This sample demonstrates integration with a microphone as the audio input source.

After building, run the command `./mic --help` for information on its command line options.

###wav

This sample demonstrates processing audio from a .wav recorded audio file.  Note: the file must contain mono audio with a 16 kHz sample rate.

After building, run the command `./wav --help` for information on its command line options.

## Dependencies

#### Affectiva Speech library (needed for both `mic` and `wav`)

The Speech Library is packaged with the Automotive SDK, which is available upon request. To get access, please [contact us](https://auto.affectiva.com/).

#### Boost (needed for both `mic` and `wav`)

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

#### PortAudio: (needed by `mic`)

See http://www.portaudio.com/

Ubuntu:
`$ sudo apt-get install portaudio19-dev`

#### LibSndFile: (needed by `wav`)

See http://www.mega-nerd.com/libsndfile/

Ubuntu:
`$ sudo apt-get install libsndfile1-dev`

#### CMake
Ubuntu:
`$ sudo apt-get install -y cmake`



## Building with CMake

Specify the the following CMake variables to identify the locations of various dependencies:

- **BUILD_MIC**: Build the `mic` sample app.
- **BUILD_WAV**: Build the `wav` sample app.
- **AFFECTIVA_SDK_DIR**: path to the folder where the Automotive SDK is installed
- **BOOST_ROOT** path to the Boost src tree

If you are building the `wav` app:
- **PortAudio_INCLUDE**: path to the folder containing the PortAudio header files
- **PortAudio_LIBRARY**: path to the release mode PortAudio library
- **PortAudio_LIBRARY_DEBUG**: path to the debug mode PortAudio library

If you are building the `mic` app:
- **LibSndFile_INCLUDE**: path to the folder containing the LibSndFile header files
- **LibSndFile_LIBRARY**: path to the release mode LibSndFile library
- **LibSndFile_LIBRARY_DEBUG**: path to the debug mode LibSndFile library

Note: for the \*\_LIBRARY and \*\_LIBRARY_DEBUG values, you only need to provide one or the other, as appropriate to the type of build you're doing (i.e. Release or Debug, as indicated by CMAKE_BUILD_TYPE).

#### Linux (x86_64, aarch64)

For building under Linux, type the following command:

`$ cmake . -DCMAKE_BUILD_TYPE=[Release,Debug] <other args>`

Example (replace directories starting with `/path/to` as appropriate and `${platform-type}` to either `x86_64` or `aarch64`):
```
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release \
-DBUILD_MIC=ON \
-DBUILD_WAV=ON \
-DAFFECTIVA_SDK_DIR=/path/to/auto-sdk \
-DBOOST_ROOT=/path/to/boost-build \
-DPortAudio_INCLUDE=/usr/include -DPortAudio_LIBRARY=/usr/lib/${platform-type}-linux-gnu/libportaudio.so.2 \
-DLibSndFile_INCLUDE=/usr/include -DLibSndFile_LIBRARY=/usr/lib/${platform-type}-linux-gnu/libsndfile.so \
-DCMAKE_INSTALL_PREFIX=/path/to/install"

# create a build directory
mkdir speech-samples-build/
cd speech-samples-build

# run cmake, specifying the path to the CMakeLists.txt file (the same directory
# that this README.md file resides in)
cmake $CMAKE_ARGS /path/to/cpp-sdk-samples/speech
make -j4
make install
```

##Docker Build Instructions

A Dockerfile is located in the top-level directory of this repo ([here](../Dockerfile)). To build the docker image, please refer to that file for instructions.
