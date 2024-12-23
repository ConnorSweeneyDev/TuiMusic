#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "application.hpp"

namespace tuim::utility
{
  void populate_playlists(std::vector<std::filesystem::path> &playlist_directories);
  float get_decibels(application::Song &song);
  std::string seconds_to_minutes(int seconds);
}
