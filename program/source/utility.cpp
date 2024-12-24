#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <stdio.h>
#include <string>
#include <vector>

#include "taglib/fileref.h"
#include "taglib/tstring.h"

#include "application.hpp"
#include "utility.hpp"

namespace tuim::utility
{
  void populate_playlists(std::vector<std::filesystem::path> &playlist_directories)
  {
    for (const auto &directory : playlist_directories)
    {
      if (!std::filesystem::is_directory(directory))
      {
        std::cout << directory << " is not a directory." << std::endl;
        exit(EXIT_FAILURE);
      }
      std::vector<application::Song> temporary_songs = {};
      for (const auto &file : std::filesystem::directory_iterator(directory))
      {
        TagLib::FileRef file_reference(file.path().string().c_str());
        if (file_reference.isNull())
        {
          std::cout << "FileRef Error: " << file << " could not be loaded." << std::endl;
          exit(1);
        }
        TagLib::String title_tag = file_reference.tag()->title();
        std::string title = title_tag.to8Bit(true);
        TagLib::String artist_tag = file_reference.tag()->artist();
        std::string artist = artist_tag.to8Bit(true);

        if (file.is_regular_file() && file.path().extension() == ".mp3")
          temporary_songs.push_back(application::Song{file.path(), title, artist});
      }
      application::Playlist new_playlist = {directory, directory.filename().string(), temporary_songs};
      application::playlists.push_back(std::make_shared<application::Playlist>(new_playlist));
    }
  }

  void write_state_file()
  {
    std::filesystem::path state_path = "user/state.txt";
    std::ofstream state_file(state_path);

    state_file << application::volume << std::endl;
    int target_playlist_index = 0;
    for (auto &playlist : application::playlists)
    {
      if (playlist == application::current_song_playlist) break;
      target_playlist_index++;
    }
    state_file << target_playlist_index << std::endl;
    state_file << application::current_song_index << std::endl;
    state_file << application::current_song_percentage << std::endl;

    state_file.close();
  }

  float get_decibels(application::Song &song)
  {
    std::string command = "ffmpeg -i \"" + song.path.string() + "\" -filter:a volumedetect -f null /dev/null 2>&1";
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
    return decibels;
  }

  std::string seconds_to_minutes(int seconds)
  {
    int minutes = seconds / 60;
    int seconds_remaining = seconds % 60;
    return std::to_string(minutes) + ":" +
           ((seconds_remaining < 10) ? "0" + std::to_string(seconds_remaining) : std::to_string(seconds_remaining));
  }

  bool is_number(std::string string)
  {
    for (auto &character : string)
      if (character < '0' || character > '9') return false;
    return true;
  }
}
