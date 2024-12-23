#include <cstdlib>
#include <iostream>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_mixer.h"

#include "application.hpp"
#include "system.hpp"

namespace tuim::system
{
  void initialize_sdl()
  {
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
      std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }
    if (Mix_Init(MIX_INIT_MP3) == 0)
    {
      std::cout << "Mix_Init Error: " << Mix_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) != 0)
    {
      std::cout << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  void cleanup()
  {
    Mix_FreeMusic(application::current_song);
    application::current_song = nullptr;
    application::current_song_index = 0;
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
  }
}
