#include <cstddef>
#include <cstdlib>
#include <iostream>

#include "SDL_mixer.h"

#include "application.hpp"
#include "input.hpp"
#include "interface.hpp"

namespace tuim::input
{
  bool menu_down(int amount, bool is_song)
  {
    if (is_song)
    {
      interface::hovered_song += amount;
      application::playlists[(size_t)application::current_playlist]->hovered_song = interface::hovered_song;
    }
    else
      interface::hovered_playlist += amount;
    return true;
  }

  bool menu_up(int amount, bool is_song)
  {
    if (is_song)
    {
      interface::hovered_song -= amount;
      application::playlists[(size_t)application::current_playlist]->hovered_song = interface::hovered_song;
    }
    else
      interface::hovered_playlist -= amount;
    return true;
  }

  bool menu_open_or_close(bool should_open)
  {
    if (should_open)
    {
      interface::playlist_menu_width = interface::playlist_menu_max_width;
      interface::playlist_menu->TakeFocus();
    }
    else
    {
      interface::playlist_menu_width = -1;
      interface::song_menu->TakeFocus();
    }
    return true;
  }

  bool menu_select(bool is_song)
  {
    if (is_song)
    {
      application::current_song_playlist = application::playlists[(size_t)application::current_playlist];
      Mix_FreeMusic(application::current_song);
      application::current_song = nullptr;
      application::Song &new_song =
        application::playlists[(size_t)application::current_playlist]->songs[(size_t)interface::hovered_song];
      application::current_song = Mix_LoadMUS(new_song.path.string().c_str());
      if (application::current_song == nullptr)
      {
        std::cout << "Mix_LoadMUS Error: " << new_song.path << ": " << Mix_GetError() << std::endl;
        exit(EXIT_FAILURE);
      }
      Mix_VolumeMusic(application::volume * (MIX_MAX_VOLUME / 100));
      Mix_PlayMusic(application::current_song, 0);
      application::current_song_display = interface::song_menu_entries[(size_t)interface::hovered_song];
      application::paused = false;
    }
    else
    {
      application::current_playlist = interface::hovered_playlist;
      interface::hovered_song = application::playlists[(size_t)application::current_playlist]->hovered_song;
      interface::song_menu_entries.clear();
      for (auto &song : application::playlists[(size_t)application::current_playlist]->songs)
        interface::song_menu_entries.push_back(song.title + " ┃ " + song.artist);
    }
    return true;
  }

  bool pause_or_play()
  {
    if (application::paused)
      Mix_ResumeMusic();
    else
      Mix_PauseMusic();
    application::paused = !application::paused;
    return true;
  }

  bool seek_forward(int percentage)
  {
    double new_position = Mix_GetMusicPosition(application::current_song) +
                          Mix_MusicDuration(application::current_song) / (double)(100.0f / (float)percentage);
    if (new_position > Mix_MusicDuration(application::current_song))
      Mix_SetMusicPosition(Mix_MusicDuration(application::current_song));
    else
      Mix_SetMusicPosition(new_position);
    return true;
  }

  bool seek_backward(int percentage)
  {
    double new_position = Mix_GetMusicPosition(application::current_song) -
                          Mix_MusicDuration(application::current_song) / (double)(100.0f / (float)percentage);
    if (new_position < 0)
      Mix_SetMusicPosition(0);
    else
      Mix_SetMusicPosition(new_position);
    return true;
  }

  bool volume_up(int amount)
  {
    application::volume += amount;
    if (application::volume > 100) application::volume = 100;
    Mix_VolumeMusic(application::volume * (MIX_MAX_VOLUME / 100));
    return true;
  }

  bool volume_down(int amount)
  {
    application::volume -= amount;
    if (application::volume < 0) application::volume = 0;
    Mix_VolumeMusic(application::volume * (MIX_MAX_VOLUME / 100));
    return true;
  }

  bool escape()
  {
    interface::screen.ExitLoopClosure()();
    return true;
  }
}