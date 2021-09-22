Vision
======

Vision is a browser-like program for visualizing the results of remote rendering
software. With Vision, you can write a simple rendering program in your language
of choice, and see the results in a browser-like tab. You have the option of
either interfacing with a TCP connection or standard input and output. This also
makes it to develop rendering algorithms in Docker, or on a remote server with
the dedicated hardware.

### Building from Scratch

You'll need Qt5, CMake, and a working C++ compiler in order to compile this
program. Once you do, you can build with with the following commands:

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

If you're on Windows and using CMake with Visual Studio (which is the default
for Windows), then you'll need to replace the `cmake` commands above with these
ones:

```
cmake.exe ..
cmake.exe --build . --config Release
```
