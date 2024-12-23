#pragma once

#include "SDL_mixer.h"
#include "ftxui/component/loop.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

int main();
std::string seconds_to_minutes(int seconds);

namespace tuim
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
  };

  void initialize_sdl();
  void initialize_playlists();
  void initialize_menus();
  void initialize_menu_keybinds();
  void initialize_containers();
  void initialize_container_keybinds();
  void initialize_renderer();
  void run_loop();
  void cleanup();

  inline std::vector<std::shared_ptr<Playlist>> playlists = {};
  inline int hovered_playlist = 0;
  inline int hovered_song = 0;
  inline int playlist_menu_width = -1;
  inline int playlist_menu_max_width = 0;

  inline int current_playlist = 0;
  inline Mix_Music *current_song = nullptr;
  inline std::string current_song_display = "None";
  inline int volume = 10;
  inline bool paused = false;

  inline ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
  inline std::vector<std::string> playlist_menu_entries = {};
  inline std::vector<std::string> song_menu_entries = {};
  inline ftxui::Component playlist_menu = {};
  inline ftxui::Component song_menu = {};
  inline ftxui::Component container = {};
  inline ftxui::Component renderer = {};
}
