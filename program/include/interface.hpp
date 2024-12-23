#pragma once

#include <string>
#include <vector>

#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"

namespace tuim::interface
{
  void initialize_menus();
  void initialize_containers();
  void initialize_renderer();

  inline int hovered_playlist = 0;
  inline int hovered_song = 0;
  inline int playlist_menu_width = -1;
  inline int playlist_menu_max_width = 0;

  inline ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
  inline std::vector<std::string> playlist_menu_entries = {};
  inline std::vector<std::string> song_menu_entries = {};
  inline ftxui::Component playlist_menu = {};
  inline ftxui::Component song_menu = {};
  inline ftxui::Component container = {};
  inline ftxui::Component renderer = {};
}
