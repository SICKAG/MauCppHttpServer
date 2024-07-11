# MauCppHttpServer

DLL wrapper for C++ HTTP server abstraction and implementations.

## Licenses

This software is under the LGPLv3. Please see the ``LICENSE`` file and the ``third-party/licenses`` folder for details.

Third-party libraries are:

* [Qt](https://www.qt.io/)
* [Boost](https://www.boost.org/)
* [OpenSSL](https://www.openssl.org/)
* [Webcc](https://github.com/sickAG/webcc/)

At build time, you also need:

* [CMake](https://cmake.org/)

## Building from source

This is how we build the DLL at SICK AG. By naming the source folder after the release, we
ensure that the paths in debug ```.PDB``` files also have the release number embedded.
This makes co-existence of several versions on target machines, with debug sources separated from each other, possible.

Of course, directories may be named differently.

Windows build instructions (other platforms are not officialy supported):

- Clone the project into ``C:\Software\External\vc17_x64\MauCppHttpServer-1.0.0`` (adjusted to the version number you intend to build).
- Check out the Git tag to be built.
- Create an empty build folder like C:\Software\External\vc17_x64\MauCppHttpServer-build (``mkdir C:\Software\External\vc17_x64\MauCppHttpServer-build``).
- Open the suitable Visual Studio Native Tools Command Prompt and change directory to the build folder.
- Execute the following CMake command, possibly entering your own dependency locations (paths here are from the SICK build system):

```
cmake ..\MauCppHttpServer-1.0.0\src -DCMAKE_INSTALL_PREFIX=C:\Software\External\vc17_x64\MauCppHttpServer-1.0.0 -DCMAKE_PREFIX_PATH=C:\Software\External\vc17_x64\qt\6.7.1 -DBoost_INCLUDE_DIR=C:\Software\External\vc17_x64\boost_1_85_0\ -DOPENSSL_ROOT_DIR=C:\Software\External\vc17_x64\openssl-3.2.1 -DWEBCCDIR=C:\Software\External\vc17_x64\webcc-1.0.1-sick
```

Note that again, we have the release version embedded in the ``CMAKE_INSTALL_PREFIX``, as we want to install the binaries
into the sources. This is not mandatory.

Finally, you may build the solution. Do not forget to build the ``INSTALL`` target as well.

## Deployment for SICK Build System

You may skip this entire section if you just intend do build the binary and replace it manually.

### Identity Files

For proper SICK build system integration, the following empty & extensionless files should be created in the directory defined by ``CMAKE_INSTALL_PREFIX`` :

- ``version_1_0_0`` (version identity; adjust version number)
- ``generator_vc17`` (compiler identity; adjust if necessary)
- ``platform_x64`` (platform architecture)

### Deploy to Artifactory

When ``.rar``-ing, exclude the ``.git`` folder!

Naming pattern: ``MauCppHttpServer-1.0.0_v17_x64.rar``

Deploy in the ``dep`` category.
