# HttpServer

DLL wrapper for C++ HTTP server abstraction and implementations.

### Dependencies

* [Qt](https://www.qt.io/)
* [Boost](https://www.boost.org/)
* [OpenSSL](https://www.openssl.org/)
* [Webcc](https://github.com/sickAG/webcc/)
* [CMake](https://cmake.org/)

## Building from source

To build, do something like this:

- Clone the project into C:\Software\External\vc17_x64\MauCppHttpServer
- Create an empty folder C:\Software\External\vc17_x64\MauCppHttpServer-build (``mkdir C:\Software\External\vc17_x64\MauCppHttpServer-build``)
- Go to the build folder (``cd C:\Software\External\vc17_x64\MauCppHttpServer-build``)
- Execute the following CMake command (using example dependency locations):

```
cmake ..\MauCppHttpServer\src -DCMAKE_INSTALL_PREFIX=C:\Software\External\vc17_x64\MauCppHttpServer -DCMAKE_PREFIX_PATH=C:\Software\External\vc17_x64\qt\6.7.1 -DBoost_INCLUDE_DIR=C:\Software\External\vc17_x64\boost_1_85_0\ -DOPENSSL_ROOT_DIR=C:\Software\External\vc17_x64\openssl-3.2.1 -DWEBCCDIR=C:\Software\External\vc17_x64\webcc-1.0.1-sick
```

Then, you may build the solution (don't forget to build the INSTALL target).