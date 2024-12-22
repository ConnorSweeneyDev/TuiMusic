#include <cctype>
#include <cmath>
#include <component/event.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_main.h"
#include "SDL_mixer.h"
#include "SDL_timer.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/loop.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/util/ref.hpp"
#include "taglib/fileref.h"
#include "taglib/tstring.h"

int main(int argc, char *argv[])
{
  if (argc > 1)
  {
    std::cout << "Usage: " << argv[0] << std::endl;
    return EXIT_FAILURE;
  }

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

  std::filesystem::path music_dir = "C:/Users/conno/Music/Songs";
  std::vector<std::filesystem::path> songs = {};
  for (const auto &entry : std::filesystem::directory_iterator(music_dir))
    if (entry.is_regular_file() && entry.path().extension() == ".mp3") songs.push_back(entry.path());

  Mix_Music *music = nullptr;
  std::vector<std::string> song_entries = {};
  for (std::filesystem::path song : songs)
  {
    TagLib::FileRef file(song.string().c_str());
    if (file.isNull())
    {
      std::cout << "FileRef Error: \"" << song << "\" could not be loaded." << std::endl;
      exit(1);
    }
    TagLib::String title_tag = file.tag()->title();
    std::string title = title_tag.to8Bit(true);
    TagLib::String artist_tag = file.tag()->artist();
    std::string artist = artist_tag.to8Bit(true);
    song_entries.push_back(title + " ┃ " + artist);
  }

  ftxui::ScreenInteractive app_screen = ftxui::ScreenInteractive::Fullscreen();
  ftxui::Screen::Cursor app_cursor;
  app_cursor.shape = ftxui::Screen::Cursor::Hidden;
  app_screen.SetCursor(app_cursor);

  std::vector<std::string> playlist_entries = {};
  for (int i = 0; i <= 5; i++) playlist_entries.push_back("playlist" + std::to_string(i));
  playlist_entries.push_back("verylongplaylist6");
  int selected_play_list = 0;
  ftxui::MenuOption play_list_option = ftxui::MenuOption::Vertical();
  play_list_option.focused_entry = ftxui::Ref<int>(&selected_play_list);
  ftxui::Component play_list = ftxui::Menu(&playlist_entries, &selected_play_list, play_list_option);
  ftxui::Component play_list_renderer = ftxui::Renderer(
    play_list, [&]
    { return ftxui::hbox({ftxui::separatorEmpty(), play_list->Render() | ftxui::yframe, ftxui::separatorEmpty()}); });

  int selected_song = 0;
  ftxui::MenuOption song_list_option = ftxui::MenuOption::Vertical();
  song_list_option.focused_entry = ftxui::Ref<int>(&selected_song);
  ftxui::Component song_list = ftxui::Menu(&song_entries, &selected_song, song_list_option);
  ftxui::Component song_list_renderer = ftxui::Renderer(
    song_list, [&]
    { return ftxui::hbox({ftxui::separatorEmpty(), song_list->Render() | ftxui::yframe, ftxui::separatorEmpty()}); });
  std::string playing_song = "None";
  bool song_paused = false;
  int volume = 10;

  play_list |= ftxui::CatchEvent(
    [&](ftxui::Event event)
    {
      if (event == ftxui::Event::j)
      {
        selected_play_list++;
        return true;
      }
      if (event == ftxui::Event::k)
      {
        selected_play_list--;
        return true;
      }
      if (event == ftxui::Event::CtrlD)
      {
        selected_play_list += 12;
        return true;
      }
      if (event == ftxui::Event::CtrlU)
      {
        selected_play_list -= 12;
        return true;
      }
      if (event == ftxui::Event::AltL)
      {
        song_list->TakeFocus();
        return true;
      }
      return false;
    });
  song_list |= ftxui::CatchEvent(
    [&](ftxui::Event event)
    {
      if (event == ftxui::Event::j)
      {
        selected_song++;
        return true;
      }
      if (event == ftxui::Event::k)
      {
        selected_song--;
        return true;
      }
      if (event == ftxui::Event::CtrlD)
      {
        selected_song += 12;
        return true;
      }
      if (event == ftxui::Event::CtrlU)
      {
        selected_song -= 12;
        return true;
      }
      if (event == ftxui::Event::Return)
      {
        Mix_FreeMusic(music);
        music = nullptr;
        music = Mix_LoadMUS(songs[(size_t)selected_song].string().c_str());
        if (music == nullptr)
        {
          std::cout << "Mix_LoadMUS Error: \"" << songs[(size_t)selected_song].string() << "\": " << Mix_GetError()
                    << std::endl;
          exit(EXIT_FAILURE);
        }
        Mix_VolumeMusic(volume * (MIX_MAX_VOLUME / 100));
        Mix_PlayMusic(music, 0);
        playing_song = song_entries[(size_t)selected_song];
        song_paused = false;
      }
      if (event == ftxui::Event::AltH)
      {
        play_list->TakeFocus();
        return true;
      }
      return false;
    });

  ftxui::Component app_container = ftxui::Container::Horizontal({play_list, song_list});
  app_container |= ftxui::CatchEvent(
    [&](ftxui::Event event)
    {
      if (event == ftxui::Event::P)
      {
        if (song_paused)
          Mix_ResumeMusic();
        else
          Mix_PauseMusic();
        song_paused = !song_paused;
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
        app_screen.ExitLoopClosure()();
        return true;
      }
      if (event.mouse().motion == ftxui::Mouse::Motion::Moved) return true;
      if (event.mouse().motion == ftxui::Mouse::Motion::Pressed) return true;
      return false;
    });
  ftxui::Component app_renderer = ftxui::Renderer(
    app_container,
    [&]
    {
      return ftxui::vbox({
               ftxui::hbox({ftxui::text((song_paused ? "⏸︎ " : "⏵︎ ") + playing_song)}) | ftxui::center,
               ftxui::hbox({ftxui::text("┃"),
                            ftxui::gaugeRight(playing_song == "None"
                                                ? 0.0f
                                                : (float)(Mix_GetMusicPosition(music) / Mix_MusicDuration(music))),
                            ftxui::text("┃ Vol: " + std::to_string(volume) +
                                        (volume < 100 ? ((volume < 10) ? "  " : " ") : ""))}),
               ftxui::separator(),
               ftxui::hbox({play_list_renderer->Render(), ftxui::separator(), song_list_renderer->Render()}),
             }) |
             ftxui::borderEmpty;
    });

  ftxui::Loop app_loop(&app_screen, app_renderer);
  while (!app_loop.HasQuitted())
  {
    if (!Mix_PlayingMusic())
    {
      Mix_FreeMusic(music);
      music = nullptr;
      playing_song = "None";
    }
    app_loop.RunOnce();
    app_screen.RequestAnimationFrame();
    SDL_Delay(10);
  }

  Mix_FreeMusic(music);
  music = nullptr;
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
