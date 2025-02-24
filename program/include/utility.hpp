#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "application.hpp"

namespace tuim::utility
{
  void populate_playlist(std::filesystem::path &playlist_directory);
  void cleanup_playlist_data();
  void write_state_file();
  float get_decibels(application::Song &song);
  std::string seconds_to_minutes(int seconds);
  int find_real_index(const std::filesystem::path &path, std::shared_ptr<application::Playlist> &playlist);
  bool is_number(std::string string);
  std::vector<std::string> split(std::string string, const std::string &delimiter);
}
