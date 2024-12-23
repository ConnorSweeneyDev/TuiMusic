#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "SDL_mixer.h"
#include "SDL_timer.h"
#include "ftxui/component/loop.hpp"
#include "taglib/fileref.h"
#include "taglib/tstring.h"

#include "application.hpp"
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
    for (const auto &directory : playlist_directories)
    {
      if (!std::filesystem::is_directory(directory))
      {
        std::cout << directory << " is not a directory." << std::endl;
        exit(EXIT_FAILURE);
      }
      std::vector<Song> temporary_songs = {};
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
          temporary_songs.push_back(Song{file.path(), title, artist});
      }
      Playlist new_playlist = {directory, directory.filename().string(), temporary_songs};
      playlists.push_back(std::make_shared<Playlist>(new_playlist));
      current_song_playlist = playlists[(size_t)current_playlist];
    }
  }

  std::string get_information_bar()
  {
    if (paused) return current_song_playlist->name + "⏸︎ " + current_song_display;
    return current_song_playlist->name + "⏵︎ " + current_song_display;
  }

  std::string get_progress_in_minutes()
  {
    if (Mix_PlayingMusic()) return utility::seconds_to_minutes((int)Mix_GetMusicPosition(current_song)) + " ┃";
    return "0:00 ┃";
  }

  float get_progress_as_percentage()
  {
    if (Mix_PlayingMusic()) return (float)(Mix_GetMusicPosition(current_song) / Mix_MusicDuration(current_song));
    return 0.0f;
  }

  std::string get_duration_in_minutes()
  {
    if (Mix_PlayingMusic()) return "| " + utility::seconds_to_minutes((int)Mix_MusicDuration(current_song));
    return "| 0:00";
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
      }
      loop.RunOnce();
      interface::screen.RequestAnimationFrame();
      SDL_Delay(10);
    }
  }
}
