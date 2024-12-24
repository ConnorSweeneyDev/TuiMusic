#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "SDL_mixer.h"
#include "SDL_timer.h"
#include "ftxui/component/loop.hpp"

#include "application.hpp"
#include "input.hpp"
#include "interface.hpp"
#include "utility.hpp"

namespace tuim::application
{
  void initialize_playlists()
  {
    std::filesystem::path playlists_path = "user/playlists.txt";
    std::ifstream playlists_file(playlists_path);
    if (!std::filesystem::exists(playlists_path) || !playlists_file.is_open())
    {
      std::cout << "Could not open " << playlists_path << "." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::vector<std::filesystem::path> playlist_directories;
    std::string line;
    while (std::getline(playlists_file, line))
    {
      if (line.length() == 0) continue;
      std::filesystem::path playlist_path = line;
      if (!std::filesystem::exists(playlist_path))
      {
        std::cout << "Playlist " << playlist_path << " does not exist." << std::endl;
        exit(EXIT_FAILURE);
      }
      playlist_directories.push_back(playlist_path);
    }
    playlists_file.close();

    if (playlist_directories.size() == 0)
    {
      std::cout << "No playlists found in " << playlists_path << "." << std::endl;
      exit(EXIT_FAILURE);
    }
    utility::populate_playlists(playlist_directories);
  }

  void initialize_state()
  {
    std::filesystem::path state_path = "user/state.txt";
    if (!std::filesystem::exists(state_path))
    {
      current_song_playlist = playlists[(size_t)current_playlist_index];
      interface::hovered_song = current_playlist_index;
      utility::write_state_file();
      return;
    }
    std::ifstream state_file(state_path);
    if (!state_file.is_open())
    {
      std::cout << "Could not open " << state_path << "." << std::endl;
      exit(EXIT_FAILURE);
    }
    std::string line;
    std::getline(state_file, line);
    volume = std::stoi(line);
    if (volume < 0 || volume > 100) volume = 10;
    bool different_song = false;
    std::getline(state_file, line);
    current_playlist_index = std::stoi(line);
    if ((size_t)current_playlist_index >= playlists.size())
    {
      different_song = true;
      current_playlist_index = 0;
    }
    current_song_playlist = playlists[(size_t)current_playlist_index];
    interface::hovered_playlist = current_playlist_index;
    std::getline(state_file, line);
    current_song_index = std::stoi(line);
    if ((size_t)current_song_index >= current_song_playlist->songs.size() || different_song)
    {
      different_song = true;
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(0, (int)current_song_playlist->songs.size() - 1);
      current_song_index = dis(gen);
    }
    interface::hovered_song = current_song_index;
    std::getline(state_file, line);
    current_song_percentage = std::stoi(line);
    if (current_song_percentage < 0 || current_song_percentage > 100 || different_song) current_song_percentage = 0;
    current_song = Mix_LoadMUS(current_song_playlist->songs[(size_t)current_song_index].path.string().c_str());
    if (current_song == nullptr)
    {
      std::cout << "Mix_LoadMUS Error: " << current_song_playlist->songs[(size_t)current_song_index].path << ": "
                << Mix_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }
    float decibels = utility::get_decibels(current_song_playlist->songs[(size_t)current_song_index]);
    volume_modifier = decibels / -14.0f;
    float real_volume = std::round(((float)volume * (MIX_MAX_VOLUME / 100.0f)) * volume_modifier);
    if (real_volume > MIX_MAX_VOLUME) real_volume = MIX_MAX_VOLUME;
    if (real_volume < 0) real_volume = 0;
    Mix_VolumeMusic((int)real_volume);
    Mix_PlayMusic(current_song, 0);
    input::seek_to(current_song_percentage);
    current_song_display = current_song_playlist->songs[(size_t)current_song_index].title + " ┃ " +
                           current_song_playlist->songs[(size_t)current_song_index].artist;
    paused = false;
    utility::write_state_file();
  }

  void play_random_song_from_playlist(std::shared_ptr<Playlist> &playlist)
  {
    Mix_FreeMusic(current_song);
    current_song = nullptr;
    current_song_index = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, (int)playlist->songs.size() - 1);
    current_song_index = dis(gen);

    int playlist_location = 0;
    for (auto &existing_playlist : playlists)
    {
      if (playlist == existing_playlist) break;
      playlist_location++;
    }
    if (playlist_location == current_playlist_index)
    {
      playlist->hovered_song = current_song_index;
      interface::hovered_song = current_song_index;
    }

    current_song_playlist = playlist;
    Song &new_song = playlist->songs[(size_t)current_song_index];
    current_song = Mix_LoadMUS(new_song.path.string().c_str());
    if (current_song == nullptr)
    {
      std::cout << "Mix_LoadMUS Error: " << new_song.path << ": " << Mix_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }

    float decibels = utility::get_decibels(new_song);
    volume_modifier = decibels / -14.0f;
    float real_volume = std::round(((float)volume * (MIX_MAX_VOLUME / 100.0f)) * volume_modifier);
    if (real_volume > MIX_MAX_VOLUME) real_volume = MIX_MAX_VOLUME;
    if (real_volume < 0) real_volume = 0;
    Mix_VolumeMusic((int)real_volume);

    Mix_PlayMusic(current_song, 0);
    current_song_display = new_song.title + " ┃ " + new_song.artist;
    paused = false;
    utility::write_state_file();
  }

  std::string get_information_bar()
  {
    if (paused) return "⏸︎ " + current_song_playlist->name + " ┃ " + current_song_display;
    return "⏵︎ " + current_song_playlist->name + " ┃ " + current_song_display;
  }

  std::string get_progress_in_minutes()
  {
    if (Mix_PlayingMusic()) return utility::seconds_to_minutes((int)Mix_GetMusicPosition(current_song)) + " ┃";
    return "-:-- ┃";
  }

  float get_progress_as_percentage()
  {
    if (Mix_PlayingMusic()) return (float)(Mix_GetMusicPosition(current_song) / Mix_MusicDuration(current_song));
    return 0.0f;
  }

  std::string get_duration_in_minutes()
  {
    if (Mix_PlayingMusic()) return "┃ " + utility::seconds_to_minutes((int)Mix_MusicDuration(current_song));
    return "┃ -:--";
  }

  std::string get_formatted_volume()
  {
    if (volume == 100) return " %" + std::to_string(volume);
    if (volume >= 10) return " %" + std::to_string(volume) + " ";
    return " %" + std::to_string(volume) + "  ";
  }

  void run_loop()
  {
    ftxui::Loop loop(&interface::screen, interface::renderer);
    while (!loop.HasQuitted())
    {
      if (!Mix_PlayingMusic())
      {
        Mix_FreeMusic(current_song);
        current_song = nullptr;
        current_song_display = "None";
        current_song_index = 0;
        current_song_percentage = 0;
        paused = false;
        play_random_song_from_playlist(current_song_playlist);
      }
      loop.RunOnce();
      interface::screen.RequestAnimationFrame();
      current_song_percentage =
        (int)std::round(((Mix_GetMusicPosition(current_song) / Mix_MusicDuration(current_song)) * 100.0));
      SDL_Delay(10);
    }
  }
}
