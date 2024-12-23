#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <regex>
#include <stdio.h>
#include <string>

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

      std::string command =
        "ffmpeg -i \"" + new_song.path.string() + "\" -filter:a volumedetect -f null /dev/null 2>&1";
      std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
      if (!pipe)
      {
        std::cout << "Failed to open pipe!" << std::endl;
        exit(EXIT_FAILURE);
      }
      float decibels = -14.0f;
      std::string ffmpeg_output;
      char buffer[128];
      while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) ffmpeg_output += buffer;
      pipe.reset();
      std::regex ffmpeg_regex("mean_volume: -?[0-9]+.[0-9]+");
      std::smatch ffmpeg_match;
      if (std::regex_search(ffmpeg_output, ffmpeg_match, ffmpeg_regex))
      {
        ffmpeg_output = ffmpeg_match[0];
        std::regex decibels_regex("-?[0-9]+.[0-9]+");
        std::smatch decibels_match;
        if (std::regex_search(ffmpeg_output, decibels_match, decibels_regex))
          decibels = std::stof(decibels_match[0]);
        else
          decibels = -14.0f;
      }
      else
        decibels = -14.0f;
      application::volume_modifier = decibels / -14.0f;
      float real_volume =
        std::round(((float)application::volume * (MIX_MAX_VOLUME / 100.0f)) * application::volume_modifier);
      if (real_volume > MIX_MAX_VOLUME) real_volume = MIX_MAX_VOLUME;
      if (real_volume < 0) real_volume = 0;
      Mix_VolumeMusic((int)real_volume);

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
    float real_volume =
      std::round(((float)application::volume * (MIX_MAX_VOLUME / 100.0f)) * application::volume_modifier);
    if (real_volume > MIX_MAX_VOLUME) real_volume = MIX_MAX_VOLUME;
    if (real_volume < 0) real_volume = 0;
    Mix_VolumeMusic((int)real_volume);
    return true;
  }

  bool volume_down(int amount)
  {
    application::volume -= amount;
    if (application::volume < 0) application::volume = 0;
    float real_volume =
      std::round(((float)application::volume * (MIX_MAX_VOLUME / 100.0f)) * application::volume_modifier);
    if (real_volume > MIX_MAX_VOLUME) real_volume = MIX_MAX_VOLUME;
    if (real_volume < 0) real_volume = 0;
    Mix_VolumeMusic((int)real_volume);
    return true;
  }

  bool escape()
  {
    interface::screen.ExitLoopClosure()();
    return true;
  }
}
