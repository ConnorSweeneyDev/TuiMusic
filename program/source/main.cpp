#include <cstdlib>
#include <fileref.h>
#include <filesystem>
#include <iostream>
#include <string>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_main.h"
#include "SDL_mixer.h"
#include "SDL_timer.h"
#include "taglib/tstring.h"

int main(int argc, char *argv[])
{
  if (argc > 1)
  {
    std::cout << "Usage: " << argv[0] << std::endl;
    return EXIT_FAILURE;
  }

  if (SDL_Init(SDL_INIT_AUDIO) != 0)
  {
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
    exit(1);
  }
  if (Mix_Init(MIX_INIT_MP3) == 0)
  {
    std::cout << "Mix_Init Error: " << Mix_GetError() << std::endl;
    exit(1);
  }
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) != 0)
  {
    std::cout << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
    exit(1);
  }

  std::filesystem::path song = std::filesystem::current_path() / "7 - Prince.mp3";
  Mix_Music *music = Mix_LoadMUS(song.string().c_str());
  if (music == nullptr)
  {
    std::cout << "Mix_LoadMUS Error: " << song << ": " << Mix_GetError() << std::endl;
    exit(1);
  }
  Mix_VolumeMusic(MIX_MAX_VOLUME / 10);
  Mix_PlayMusic(music, 0);
  TagLib::FileRef file(song.string().c_str());
  if (file.isNull())
  {
    std::cout << "TagLib::FileRef Error: \"" << song << "\" could not be loaded." << std::endl;
    exit(1);
  }
  TagLib::String title_tag = file.tag()->title();
  std::string title_str = title_tag.to8Bit(true);
  TagLib::String artist_tag = file.tag()->artist();
  std::string artist_str = artist_tag.to8Bit(true);
  std::cout << "Playing: " << title_str << " - " << artist_str << std::endl;

  while (Mix_PlayingMusic()) { SDL_Delay(100); }

  Mix_FreeMusic(music);
  music = nullptr;
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
