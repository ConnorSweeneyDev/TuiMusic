# Building and Executing
This project is optimized to be built with the following targets in mind:
- Windows 11 MinGW 64-bit GCC 14.2.0
- Ubuntu 18.04 GLIBC Version 2.27

Version information for dependencies can be found in `external/version_info.txt`.

On both Windows and Linux the binary is statically linked to all libraries.

After following the platform specific instructions below you can execute `script/build.sh` followed by `script/run.sh`
from the root of the project to build and run the project.

### Windows
Do the following to ensure your environment is set up correctly:
- Download a 64-bit [MinGW](https://winlibs.com/) distribution with Clang/LLVM support and put the `[DISTRIBUTION]/bin`
  directory in your path.
- Install GNUMake by running `winget install ezwinports.make`.
- Ensure that you have `[GIT_INSTALLATION]/bin` in your path.

### Linux
Do the following on Ubuntu to ensure your environment is set up correctly:
- Only run `sudo apt update && sudo apt upgrade` if you haven't already.
- Run `sudo apt install git g++ gdb make`.

# Updating ftxui
Since the library files are all within the project, to update ftxui for each platform some extra steps are required. The
releases can be found [here](https://github.com/ArthurSonzogni/ftxui/releases).

### Windows
On top of the previous windows setup, follow these steps to build ftxui for MinGW:
- `git clone https://github.com/ArthurSonzogni/FTXUI.git && cd FTXUI && mkdir build`.
- `cmake -B build -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"`.
- `cmake --build build --config Release`.

Now you will have access to some important folders:
- `include/` contains the header files that can replace the ones in the `external/include/ftxui` folder of this project.
  After replacing the contents of that folder, you have to remove all instances of `ftxui/` from the include paths
  within the new header files.
- `build/` contains `libftxui-component.a`, `libftxui-dom.a`, and `libftxui-screen.a` which replace the contents of
  `external/library/ftxui/windows` in this project.

### Linux
On top of the previous linux setup, you can go to the releases page and download the release ending in `Linux.tar.gz`.
After extracting the archive, you will have access to some important folders:
- `include/` contains the header files that can replace the ones in the `external/include/ftxui` folder of this project.
  After replacing the contents of that folder, you have to remove all instances of `ftxui/` from the include paths
  within the new header files.
- `lib/` contains `libftxui-component.a`, `libftxui-dom.a`, and `libftxui-screen.a` which replace the contents of
  `external/library/ftxui/linux` in this project.
