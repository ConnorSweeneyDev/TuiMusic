# TuiMusic

https://github.com/user-attachments/assets/87bf883f-6ad6-49fc-ba6c-ccc36f908da1

A terminal-based music player written using SDL, FTXUI and TagLib that has playlists and normalizes volume around -14dB.
Playlists are in the form of folders on your filesystem - place absolute paths to folders in the `user/playlists.txt`
directory and the player will load them on startup. The player will start playing a random song from the first playlist
and will continue to pick random songs from that playlist after it finishes playing the current one. If you manually
change the playlist the player will start playing a random song from the new playlist instead. State is tracked in the
`user/state.txt` file.

> [!NOTE]
> Controls are as follows:
> - <kbd>j</kbd> or <kbd>k</kbd> to navigate the focused menu by 1 item.
> - <kbd>J</kbd> or <kbd>K</kbd> to navigate the focused menu by 12 items.
> - <kbd>B</kbd> or <kbd>T</kbd> to navigate to the top/bottom of the focused menu.
> - <kbd>return</kbd> to select the focused menu item.
> - <kbd>s</kbd> to shuffle the current/focused playlist.
> - <kbd>p</kbd> to pause/unpause.
> - <kbd>h</kbd> or <kbd>l</kbd> to open/close the playlists tab.
> - <kbd>H</kbd> or <kbd>L</kbd> to seek forward/backward by 5%.
> - <kbd>0</kbd>, <kbd>1</kbd>... <kbd>9</kbd> to seek to 0%, 10%... 90% of the song.
> - <kbd>n</kbd> to end the current song.
> - <kbd>u</kbd> or <kbd>d</kbd> to increase/decrease the volume by 1%.
> - <kbd>U</kbd> or <kbd>D</kbd> to increase/decrease the volume by 5%.
> - <kbd>escape</kbd> to close the player.

# Building and Executing
This project is optimized to be built with the following targets in mind:
- Windows 11 MinGW 64-bit GCC 14.2.0
- Ubuntu 22.04 GLIBC Version 2.35

Version information for dependencies can be found in `external/version_info.txt`.

On Windows the binary is statically linked to all system libraries, but dynamically linked to SDL, SDL_mixer and ftxui.
On linux the binary is statically linked where possible (only libstdc++ and libgcc) and dynamically linked to everything
else.

After following the platform specific instructions below you can execute `script/build.sh` followed by `script/run.sh`
(or `script/run.bat` on Windows) from the root of the project to build and run it.

### Windows
Do the following to ensure your environment is set up correctly:
- Download a 64-bit [MinGW](https://winlibs.com/) distribution with Clang/LLVM support and put the `[DISTRIBUTION]/bin`
  directory in your path.
- Install GNUMake by running `winget install ezwinports.make`.
- Install ffmpeg by running `winget install Gyan.FFmpeg` (optional for volume normalization).
- Ensure that you have `[GIT_INSTALLATION]/bin` in your path.

### Linux
Do the following on Ubuntu to ensure your environment is set up correctly:
- Only run `sudo apt update && sudo apt upgrade` if you haven't already.
- Run `sudo apt install git g++ gdb make`.
- Run `sudo apt install ffmpeg` (optional for volume normalization).

After building, do the following to ensure your environment is set up correctly:
- Only run `sudo apt install alsa xorg openbox` if you don't already have an audio and window manager.

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
- `FTXUI/include/ftxui` contains the header files that can replace the ones in the `external/include/ftxui` folder of
  this project. After replacing the contents of that folder, you have to remove all instances of `ftxui/` from the
  include paths within the new header files.
- `FTXUI/build` contains the files to replace the `external/library/ftxui/windows` and `binary/windows` folders of
  this project.

### Linux
On top of the previous Linux setup, follow these steps to build ftxui for Linux:
- Ensure that you have cmake installed, if not run `sudo apt install cmake`.
- `git clone https://github.com/ArthurSonzogni/FTXUI.git && cd FTXUI && mkdir build`.
- `cmake -B build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles"`.
- `cmake --build build --config Release`.

Now you will have access to some important folders:
- `FTXUI/include/ftxui` contains the header files that can replace the ones in the `external/include/ftxui` folder of
  this project. After replacing the contents of that folder, you have to remove all instances of `ftxui/` from the
  include paths within the new header files.
- `FTXUI/build` contains the files to replace the `external/library/ftxui/linux` and `binary/linux` folders of this
  project.

## SDL
### Windows
On top of the previous Windows setup, go to the [releases](https://github.com/libsdl-org/SDL/releases) page and download
the file ending `mingw.zip`. Extract this and go to `x86_64-w64-mingw32` and you will have access to three important
folders:
- `x86_64-w64-mingw32/include/SDL2` which contains files that can replace the contents of the
  `external/include/sdl/windows` folder of this project.
- `x86_64-w64-mingw32/lib` which contains the files to replace the contents of the `external/library/sdl/windows` folder
  of this project.
- `x86_64-w64-mingw32/bin` which contains the file that can replace the one in the `binary/windows` folder of this
  project.

### Linux
On top of the previous Linux setup, do the following to ensure your environment is set up correctly:
- Only run `sudo sed -i~orig -e 's/# deb-src/deb-src/' /etc/apt/sources.list` if you haven't already.
- Only run `sudo apt update` if you just ran the previous command.
- Run `sudo apt build-dep libsdl2-dev`.

Now you can go to the [releases](https://github.com/libsdl-org/SDL/releases) page and download the
`SDL2-[VERSION].tar.gz` file. Then run the following commands:
- `tar -xvzf SDL2-[VERSION].tar.gz`
- `cd SDL2-[VERSION] && mkdir build && cd build`
- `../configure`
- `make`

Now you have two important directories:
- `SDL2-[VERSION]/include` which contains the files that can replace the ones in the `external/include/sdl/linux` folder
  of this project.
- `SDL2-[VERSION]/build/build/.libs` which contains the files that can replace the contents of the
  `external/library/sdl2/linux` and `binary/linux` folders of this project. 

## SDL_mixer
### Windows
On top of the previous Windows setup, go to the [releases](https://github.com/libsdl-org/SDL_mixer/releases) page and
download the file ending `mingw.zip`. Extract this and go to `x86_64-w64-mingw32` and you will have access to three
important folders:
- `x86_64-w64-mingw32/include/SDL2` which contains files that can replace the contents of the
  `external/include/sdl/windows` folder of this project.
- `x86_64-w64-mingw32/lib` which contains the files to replace the contents of the `external/library/sdl/windows` folder
  of this project.
- `x86_64-w64-mingw32/bin` which contains the file that can replace the one in the `binary/windows` folder of this
  project.

### Linux
On top of the previous Linux setup and the SDL setup, do the following to ensure your environment is set up correctly:
- Go to the `SDL2-[VERSION]/build` folder and run `sudo make install`.
- Run `sudo apt build-dep libsdl2-mixer-dev`.

Now you can go to the [releases](https://github.com/libsdl-org/SDL_mixer/releases) page and download the
`SDL2_mixer-[VERSION].tar.gz` file. Then run the following commands:
- `tar -xvzf SDL2_mixer-[VERSION].tar.gz`
- `cd SDL2_mixer-[VERSION] && mkdir build && cd build`
- `../configure`
- `make`

Now you have two important directories:
- `SDL2_mixer-[VERSION]/include` which contains the file that can replace the one in the `external/include/sdl/linux`
  folder of this project.
- `SDL2_mixer-[VERSION]/build/build/.libs` which contains the files that can replace the contents of the
  `external/library/sdl/linux` and `binary/linux` folders of this project.

## TagLib
### Windows
On top of the previous Windows setup, do the following:
- Run `git clone https://github.com/taglib/taglib.git && cd taglib && git submodule update --init`
- Run `cmake -B . -DBUILD_SHARED_LIBS=ON -DVISIBILITY_HIDDEN=ON -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON -DWITH_ZLIB=OFF
  -DCMAKE_BUILD_TYPE=Release -G 'MinGW Makefiles'`
- Run `cmake --build . --config Release`

Now you have one important folder; `taglib/taglib` contains the `.dll` and `.dll.a` files that can go into the
`binary/windows` and `external/library/taglib/windows` folders respectively, and it also contains the `.h` and `.tcc`
files that can go into the `external/include/taglib` folder. The `.h` and `.tcc` files are spread around not just in
this folder, but also all of it's subfolders.

### Linux
On top of the previous Linux setup, do the following:
- Run `git clone https://github.com/taglib/taglib.git && cd taglib && git submodule update --init`
- Run `cmake -B . -DBUILD_SHARED_LIBS=ON -DVISIBILITY_HIDDEN=ON -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON -DWITH_ZLIB=OFF
  -DCMAKE_BUILD_TYPE=Release -G 'Unix Makefiles'`
- Run `cmake --build . --config Release`

Now you have one important folder; `taglib/taglib` contains the `.so.[VERSION]` and `.so` files that can go into the
`binary/linux` and `external/library/taglib/linux` folders respectively, and it also contains the `.h` and `.tcc` files
that can go into the `external/include/taglib` folder. The `.h` and `.tcc` files are spread around not just in this
folder, but also all of it's subfolders.
