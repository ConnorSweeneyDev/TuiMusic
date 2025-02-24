#include <algorithm>
#include <cctype>
#include <cstdio>
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
  void populate_playlist(std::filesystem::path &playlist_directory)
  {
    std::string playlist_name = playlist_directory.filename().string();
    std::filesystem::path playlist_data_path = "user/" + playlist_name + "_playlist.txt";
    if (std::filesystem::exists(playlist_data_path))
    {
      std::vector<application::Song> temporary_songs;
      std::ifstream playlist_data_file(playlist_data_path);
      std::string line;
      while (std::getline(playlist_data_file, line))
      {
        std::vector<std::string> song_info = split(line, "|||");
        temporary_songs.push_back(application::Song{song_info[0], song_info[1], song_info[2], std::stoi(song_info[3])});
      }
      application::Playlist new_playlist = {playlist_directory, playlist_data_path,
                                            playlist_directory.filename().string(), temporary_songs};
      application::playlists.push_back(std::make_shared<application::Playlist>(new_playlist));
      return;
    }

    if (!std::filesystem::is_directory(playlist_directory))
    {
      std::cout << playlist_directory << " is not a directory." << std::endl;
      exit(EXIT_FAILURE);
    }
    std::vector<application::Song> temporary_songs = {};
    for (const auto &file : std::filesystem::directory_iterator(playlist_directory))
    {
      if (file.path().extension() != ".mp3" || !file.is_regular_file()) continue;

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
      if (title.empty() || artist.empty())
      {
        title = file.path().filename().string();
        artist = "";
      }

      temporary_songs.push_back(application::Song{file.path(), title, artist, 0});
    }
    std::sort(
      temporary_songs.begin(), temporary_songs.end(),
      [](const application::Song &a, const application::Song &b)
      {
        std::string lowercase_title_a = a.title;
        std::string lowercase_artist_a = a.artist;
        std::string lowercase_title_b = b.title;
        std::string lowercase_artist_b = b.artist;
        std::transform(lowercase_title_a.begin(), lowercase_title_a.end(), lowercase_title_a.begin(), tolower);
        std::transform(lowercase_artist_a.begin(), lowercase_artist_a.end(), lowercase_artist_a.begin(), tolower);
        std::transform(lowercase_title_b.begin(), lowercase_title_b.end(), lowercase_title_b.begin(), tolower);
        std::transform(lowercase_artist_b.begin(), lowercase_artist_b.end(), lowercase_artist_b.begin(), tolower);
        return lowercase_artist_a < lowercase_artist_b ||
               (lowercase_artist_a == lowercase_artist_b && lowercase_title_a < lowercase_title_b) ||
               (lowercase_artist_a == lowercase_artist_b && lowercase_title_a == lowercase_title_b &&
                a.path.string() < b.path.string());
      });
    application::Playlist new_playlist = {playlist_directory, playlist_data_path,
                                          playlist_directory.filename().string(), temporary_songs};
    application::playlists.push_back(std::make_shared<application::Playlist>(new_playlist));

    std::ofstream playlist_data_file(playlist_data_path);
    for (auto &song : temporary_songs)
      playlist_data_file << song.path.string() + "|||" + song.title + "|||" + song.artist + "|||0" << std::endl;
    playlist_data_file.close();
  }

  void cleanup_playlist_data()
  {
    std::filesystem::path user_path("user/");
    std::regex pattern(".*_playlist\\.txt$");
    for (const auto &file : std::filesystem::directory_iterator(user_path))
    {
      std::string file_name = file.path().filename().string();
      if (std::regex_match(file_name, pattern)) std::filesystem::remove(file.path());
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
    state_file << application::current_song_playlist->name << std::endl;
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

  int find_real_index(const std::filesystem::path &path, std::shared_ptr<application::Playlist> &playlist)
  {
    std::ifstream playlist_data_file_read(playlist->data_path);
    if (!playlist_data_file_read.is_open())
    {
      std::cout << "Could not open " << playlist->data_path << "." << std::endl;
      exit(EXIT_FAILURE);
    }
    std::string line;
    int index = 0;
    while (std::getline(playlist_data_file_read, line))
    {
      std::vector<std::string> song_info = split(line, "|||");
      if (song_info[0] == path.string()) break;
      index++;
    }
    playlist_data_file_read.close();
    line.clear();

    playlist_data_file_read.open(playlist->data_path);
    std::filesystem::path temp_path = playlist->data_path.string() + ".tmp";
    std::ofstream playlist_data_file_write(temp_path);
    if (!playlist_data_file_read.is_open() || !playlist_data_file_write.is_open())
    {
      std::cout << "Could not open " << playlist->data_path << "." << std::endl;
      exit(EXIT_FAILURE);
    }
    int current_index = 0;
    while (std::getline(playlist_data_file_read, line))
    {
      if (current_index != index)
      {
        playlist_data_file_write << line << std::endl;
        current_index++;
        continue;
      }
      std::vector<std::string> song_info = split(line, "|||");
      std::string num_plays = std::to_string(std::stoi(song_info[3]) + 1);
      playlist_data_file_write << song_info[0] + "|||" + song_info[1] + "|||" + song_info[2] + "|||" + num_plays
                               << std::endl;
      current_index++;
    }
    playlist_data_file_read.close();
    playlist_data_file_write.close();
    std::remove(playlist->data_path.string().c_str());
    std::rename(temp_path.string().c_str(), playlist->data_path.string().c_str());

    return index;
  }

  bool is_number(std::string string)
  {
    for (auto &character : string)
      if (character < '0' || character > '9') return false;
    return true;
  }

  std::vector<std::string> split(std::string string, const std::string &delimiter)
  {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = string.find(delimiter)) != std::string::npos)
    {
      token = string.substr(0, pos);
      tokens.push_back(token);
      string.erase(0, pos + delimiter.length());
    }
    tokens.push_back(string);
    return tokens;
  }
}
