#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "SDL_mixer.h"

namespace tuim::application
{
  struct Song
  {
    std::filesystem::path path;
    std::string title;
    std::string artist;
  };
  struct Playlist
  {
    std::filesystem::path path;
    std::string name;
    std::vector<Song> songs;
    int hovered_song = 0;
  };

  inline std::vector<std::shared_ptr<Playlist>> playlists = {};
  inline int current_playlist_index = 0;
  inline int current_song_index = 0;
  inline Mix_Music *current_song = nullptr;
  inline std::shared_ptr<Playlist> current_song_playlist = nullptr;
  inline std::string current_song_display = "None";
  inline int volume = 10;
  inline float volume_modifier = 1.0f;
  inline bool paused = false;

  void initialize_playlists();
  void initialize_volume();
  void play_random_song_from_playlist(std::shared_ptr<Playlist> &playlist);
  std::string get_information_bar();
  std::string get_progress_in_minutes();
  float get_progress_as_percentage();
  std::string get_duration_in_minutes();
  std::string get_formatted_volume();
  void run_loop();
}
