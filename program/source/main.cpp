#include <cctype>
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

  std::filesystem::path song = std::filesystem::current_path() / "7 - Prince.mp3";
  Mix_Music *music = Mix_LoadMUS(song.string().c_str());
  if (music == nullptr)
  {
    std::cout << "Mix_LoadMUS Error: \"" << song << "\": " << Mix_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }
  Mix_VolumeMusic(MIX_MAX_VOLUME / 10);
  Mix_PlayMusic(music, 0);

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

  auto screen = ftxui::ScreenInteractive::Fullscreen();
  ftxui::Screen::Cursor cursor;
  cursor.shape = ftxui::Screen::Cursor::Hidden;
  screen.SetCursor(cursor);

  std::vector<std::string> playlist_entries = {};
  for (int i = 0; i <= 5; i++) playlist_entries.push_back("playlist" + std::to_string(i));
  playlist_entries.push_back("veryveryverylongplaylist6");
  int selected_play_list = 0;
  ftxui::MenuOption play_list_option = ftxui::MenuOption::Vertical();
  play_list_option.focused_entry = ftxui::Ref<int>(&selected_play_list);
  ftxui::Component play_list = ftxui::Menu(&playlist_entries, &selected_play_list, play_list_option);
  ftxui::Component play_list_renderer = Renderer(
    play_list, [&]
    { return ftxui::hbox({ftxui::separatorEmpty(), play_list->Render() | ftxui::yframe, ftxui::separatorEmpty()}); });

  std::vector<std::string> song_entries = {title + " - " + artist};
  for (int i = 0; i <= 49; i++) song_entries.push_back("song" + std::to_string(i));
  song_entries.push_back("veryveryverylongsong50");
  int selected_song = 0;
  ftxui::MenuOption song_list_option = ftxui::MenuOption::Vertical();
  song_list_option.focused_entry = ftxui::Ref<int>(&selected_song);
  ftxui::Component song_list = ftxui::Menu(&song_entries, &selected_song, song_list_option);
  ftxui::Component song_list_renderer = Renderer(
    song_list, [&]
    { return ftxui::hbox({ftxui::separatorEmpty(), song_list->Render() | ftxui::yframe, ftxui::separatorEmpty()}); });

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
      if (event == ftxui::Event::AltH)
      {
        play_list->TakeFocus();
        return true;
      }
      return false;
    });

  ftxui::Component song_container = ftxui::Container::Vertical({
    play_list,
    song_list,
  });
  song_container |= ftxui::CatchEvent(
    [&](ftxui::Event event)
    {
      if (event == ftxui::Event::Escape)
      {
        screen.ExitLoopClosure()();
        return true;
      }
      return false;
    });
  ftxui::Component renderer =
    Renderer(song_container,
             [&]
             {
               return ftxui::hbox({play_list_renderer->Render(), ftxui::separator(), song_list_renderer->Render()}) |
                      ftxui::borderHeavy;
             });

  ftxui::Loop loop(&screen, renderer);
  while (!loop.HasQuitted())
  {
    if (!Mix_PlayingMusic()) { screen.ExitLoopClosure()(); }
    loop.RunOnce();
    SDL_Delay(10);
  }

  Mix_FreeMusic(music);
  music = nullptr;
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
