# Building and Executing
This project is optimized to be built with the following targets in mind:
- Windows 11 MinGW 64-bit GCC 14.2.0
- Ubuntu 22.04 GLIBC Version 2.35

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

# Updating Libraries
Since the library files are all within the project, to update the libraries for each platform some extra steps are
required.

## ftxui
### Windows
On top of the previous Windows setup, follow these steps to build ftxui for MinGW:
- Ensure that you have cmake installed, if not run `winget install Kitware.CMake`.
- `git clone https://github.com/ArthurSonzogni/FTXUI.git && cd FTXUI && mkdir build`.
- `cmake -B build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"`.
- `cmake --build build --config Release`.

Now you will have access to some important folders:
- `include/` contains the header files that can replace the ones in the `external/include/ftxui` folder of this project.
  After replacing the contents of that folder, you have to remove all instances of `ftxui/` from the include paths
  within the new header files.
- `build/` contains `libftxui-component.dll`, `libftxui-dom.dll`, and `libftxui-screen.dll` which replace the contents
  of `external/library/ftxui/windows` in this project.

### Linux
On top of the previous Linux setup, follow these steps to build ftxui for Linux:
- Ensure that you have cmake installed, if not run `sudo apt install cmake`.
- `git clone https://github.com/ArthurSonzogni/FTXUI.git && cd FTXUI && mkdir build`.
- `cmake -B build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles"`.
- `cmake --build build --config Release`.

Now you will have access to some important folders:
- `include/` contains the header files that can replace the ones in the `external/include/ftxui` folder of this project.
  After replacing the contents of that folder, you have to remove all instances of `ftxui/` from the include paths
  within the new header files.
- `build/` contains `libftxui-component.so`, `libftxui-dom.so`, and `libftxui-screen.so` which replace the contents of
  `external/library/ftxui/linux` in this project.

## SDL
### Windows
On top of the previous windows setup, go to the [releases](https://github.com/libsdl-org/SDL/releases) page and download
the file ending `mingw.zip`. Extract this and go to `x86_64-w64-mingw32` and you will have access to three important
folders:
- `bin` which contains the file that can replace the one in the `binary/windows` folder of this project.
- `include/SDL2` which contains files that can replace the contents of the `external/include/sdl/windows` folder of this
  project.
- `lib` which contains the files (not the folders) to replace the contents of the `external/library/sdl2/windows` folder
  of this project.

### Linux
Soon.

## SDL_mixer
### Windows
On top of the previous windows setup, go to the [releases](https://github.com/libsdl-org/SDL_mixer/releases) page and
download the file ending `mingw.zip`. Extract this and go to `x86_64-w64-mingw32` and you will have access to three
important folders:
- `bin` which contains the file that can replace the one in the `binary/windows` folder of this project.
- `include/SDL2` which contains files that can replace the contents of the `external/include/sdl_mixer/windows` folder of
  this project.
- `lib` which contains the files (not the folders) to replace the contents of the `external/library/sdl_mixer/windows`
  folder of this project.

### Linux
Soon.
