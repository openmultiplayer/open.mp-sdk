# open.mp SDK

## Tools

* [CMake 3.19+](https://cmake.org/)
* Nix

## Tools on Windows

* [Visual Studio 2019+](https://www.visualstudio.com/)

Visual Studio needs the `Desktop development with C++` workload with the `MSVC v142`, `Windows 10 SDK` and `C++ Clang tools for Windows` components.

## Building on Windows

```bash
cd open.mp-sdk
mkdir build
cd build
cmake .. -A Win32 -T ClangCL
```

Open Visual Studio and build the solution.
