#define SDL_MAIN_HANDLED

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_mixer.h"
#include "SDL_timer.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/loop.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/util/ref.hpp"
#include "taglib/fileref.h"
#include "taglib/tstring.h"

#include "main.hpp"

int main()
{
  tuim::initialize_sdl();
  tuim::initialize_playlists();
  tuim::initialize_menus();
  tuim::initialize_menu_keybinds();
  tuim::initialize_containers();
  tuim::initialize_container_keybinds();
  tuim::initialize_renderer();

  tuim::run_loop();

  tuim::cleanup();
  return EXIT_SUCCESS;
}

std::string seconds_to_minutes(int seconds)
{
  int minutes = seconds / 60;
  int seconds_remaining = seconds % 60;
  return std::to_string(minutes) + ":" +
         ((seconds_remaining < 10) ? "0" + std::to_string(seconds_remaining) : std::to_string(seconds_remaining));
}

namespace tuim
{
  void initialize_sdl()
  {
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
      std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }
    if (Mix_Init(MIX_INIT_MP3) == 0)
    {
      std::cout << "Mix_Init Error: " << Mix_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) != 0)
    {
      std::cout << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
      exit(EXIT_FAILURE);
    }
  }

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
    }
  }

  void initialize_menus()
  {
    ftxui::Screen::Cursor cursor;
    cursor.shape = ftxui::Screen::Cursor::Hidden;
    screen.SetCursor(cursor);

    for (auto &playlist : playlists)
      if ((int)playlist->name.length() > playlist_menu_max_width)
        playlist_menu_max_width = (int)playlist->name.length();
    playlist_menu_max_width += 3;
    for (auto &playlist : playlists) playlist_menu_entries.push_back(playlist->name);
    ftxui::MenuOption playlist_menu_option = ftxui::MenuOption::Vertical();
    playlist_menu_option.focused_entry = ftxui::Ref<int>(&hovered_playlist);
    playlist_menu = ftxui::Menu(&playlist_menu_entries, &hovered_playlist, playlist_menu_option);

    for (auto &song : playlists[(size_t)current_playlist]->songs)
      song_menu_entries.push_back(song.title + " ┃ " + song.artist);
    ftxui::MenuOption song_menu_option = ftxui::MenuOption::Vertical();
    song_menu_option.focused_entry = ftxui::Ref<int>(&hovered_song);
    song_menu = ftxui::Menu(&song_menu_entries, &hovered_song, song_menu_option);
  }

  void initialize_menu_keybinds()
  {
    playlist_menu |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (event == ftxui::Event::j)
        {
          hovered_playlist++;
          return true;
        }
        if (event == ftxui::Event::k)
        {
          hovered_playlist--;
          return true;
        }
        if (event == ftxui::Event::CtrlD)
        {
          hovered_playlist += 12;
          return true;
        }
        if (event == ftxui::Event::CtrlU)
        {
          hovered_playlist -= 12;
          return true;
        }
        if (event == ftxui::Event::AltL)
        {
          playlist_menu_width = -1;
          song_menu->TakeFocus();
          return true;
        }
        if (event == ftxui::Event::Return)
        {
          current_playlist = hovered_playlist;
          hovered_song = playlists[(size_t)current_playlist]->hovered_song;
          song_menu_entries.clear();
          for (auto &song : playlists[(size_t)current_playlist]->songs)
            song_menu_entries.push_back(song.title + " ┃ " + song.artist);
          return true;
        }
        return false;
      });

    song_menu |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (event == ftxui::Event::j)
        {
          hovered_song++;
          playlists[(size_t)current_playlist]->hovered_song = hovered_song;
          return true;
        }
        if (event == ftxui::Event::k)
        {
          hovered_song--;
          playlists[(size_t)current_playlist]->hovered_song = hovered_song;
          return true;
        }
        if (event == ftxui::Event::CtrlD)
        {
          hovered_song += 12;
          playlists[(size_t)current_playlist]->hovered_song = hovered_song;
          return true;
        }
        if (event == ftxui::Event::CtrlU)
        {
          hovered_song -= 12;
          playlists[(size_t)current_playlist]->hovered_song = hovered_song;
          return true;
        }
        if (event == ftxui::Event::AltH)
        {
          playlist_menu_width = playlist_menu_max_width;
          playlist_menu->TakeFocus();
          return true;
        }
        if (event == ftxui::Event::Return)
        {
          Mix_FreeMusic(current_song);
          current_song = nullptr;
          Song &new_song = playlists[(size_t)current_playlist]->songs[(size_t)hovered_song];
          current_song = Mix_LoadMUS(new_song.path.string().c_str());
          if (current_song == nullptr)
          {
            std::cout << "Mix_LoadMUS Error: " << new_song.path << ": " << Mix_GetError() << std::endl;
            exit(EXIT_FAILURE);
          }
          Mix_VolumeMusic(volume * (MIX_MAX_VOLUME / 100));
          Mix_PlayMusic(current_song, 0);
          current_song_display = song_menu_entries[(size_t)hovered_song];
          paused = false;
          return true;
        }
        return false;
      });
  }

  void initialize_containers()
  {
    container =
      ftxui::ResizableSplitLeft(playlist_menu | ftxui::yframe, song_menu | ftxui::yframe, &playlist_menu_width);
  }

  void initialize_container_keybinds()
  {
    container |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (event == ftxui::Event::P)
        {
          if (paused)
            Mix_ResumeMusic();
          else
            Mix_PauseMusic();
          paused = !paused;
          return true;
        }
        if (event == ftxui::Event::L)
        {
          double new_position = Mix_GetMusicPosition(current_song) + Mix_MusicDuration(current_song) / 20;
          if (new_position > Mix_MusicDuration(current_song))
            Mix_SetMusicPosition(Mix_MusicDuration(current_song));
          else
            Mix_SetMusicPosition(new_position);
          return true;
        }
        if (event == ftxui::Event::H)
        {
          double new_position = Mix_GetMusicPosition(current_song) - Mix_MusicDuration(current_song) / 20;
          if (new_position < 0)
            Mix_SetMusicPosition(0);
          else
            Mix_SetMusicPosition(new_position);
          return true;
        }
        if (event == ftxui::Event::U)
        {
          if (volume < 100) volume++;
          Mix_VolumeMusic(volume * (MIX_MAX_VOLUME / 100));
          return true;
        }
        if (event == ftxui::Event::D)
        {
          if (volume > 0) volume--;
          Mix_VolumeMusic(volume * (MIX_MAX_VOLUME / 100));
          return true;
        }
        if (event == ftxui::Event::Escape)
        {
          screen.ExitLoopClosure()();
          return true;
        }
        if (event.mouse().motion == ftxui::Mouse::Motion::Moved) return true;
        if (event.mouse().motion == ftxui::Mouse::Motion::Pressed) return true;
        return false;
      });
  }

  void initialize_renderer()
  {
    renderer = ftxui::Renderer(
      container,
      [&]
      {
        return ftxui::vbox({
                 ftxui::hbox({ftxui::text((paused ? "⏸︎ " : "⏵︎ ") + current_song_display)}) | ftxui::center,
                 ftxui::hbox(
                   {ftxui::text(Mix_PlayingMusic() ? seconds_to_minutes((int)Mix_GetMusicPosition(current_song)) + " ┃"
                                                   : "0:00 ┃"),
                    ftxui::gaugeRight(Mix_PlayingMusic()
                                        ? (float)(Mix_GetMusicPosition(current_song) / Mix_MusicDuration(current_song))
                                        : 0.0f),
                    ftxui::text(
                      "┃ " + (Mix_PlayingMusic() ? seconds_to_minutes((int)Mix_MusicDuration(current_song)) : "0:00")),
                    ftxui::text(" %" + std::to_string(volume) + (volume < 100 ? ((volume < 10) ? "  " : " ") : ""))}),
                 ftxui::separator(),
                 container->Render(),
               }) |
               ftxui::borderEmpty;
      });
    song_menu->TakeFocus();
  }

  void run_loop()
  {
    ftxui::Loop loop(&screen, renderer);
    while (!loop.HasQuitted())
    {
      if (!Mix_PlayingMusic())
      {
        Mix_FreeMusic(current_song);
        current_song = nullptr;
        current_song_display = "None";
      }
      loop.RunOnce();
      screen.RequestAnimationFrame();
      SDL_Delay(10);
    }
  }

  void cleanup()
  {
    Mix_FreeMusic(current_song);
    current_song = nullptr;
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
  }
}
