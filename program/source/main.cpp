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

  std::vector<std::string> entries = {title + " - " + artist,
                                      "test1",
                                      "test2",
                                      "test3",
                                      "test4",
                                      "test5",
                                      "test6",
                                      "test7",
                                      "test8",
                                      "test9",
                                      "test10",
                                      "test11",
                                      "test12",
                                      "test13",
                                      "test14",
                                      "test15",
                                      "test16",
                                      "test17",
                                      "test18",
                                      "test19",
                                      "test20",
                                      "test21",
                                      "test22",
                                      "test23",
                                      "test24",
                                      "test25",
                                      "test26",
                                      "test27",
                                      "test28",
                                      "test29",
                                      "test30",
                                      "test31",
                                      "test32",
                                      "test33",
                                      "test34",
                                      "test35",
                                      "test36",
                                      "test37",
                                      "test38",
                                      "test39",
                                      "test40",
                                      "test41",
                                      "test42",
                                      "test43",
                                      "test44",
                                      "test45",
                                      "test46",
                                      "test47",
                                      "test48",
                                      "test49",
                                      "longlonglongtest50"};
  int selected = 0;
  ftxui::MenuOption menu_option;
  menu_option.on_enter = screen.ExitLoopClosure();
  ftxui::Component song_list = Menu(&entries, &selected, menu_option);

  auto container = ftxui::Container::Vertical({
    song_list,
  });

  auto renderer = Renderer(container,
                           [&]
                           {
                             return ftxui::hbox({
                                      song_list->Render() | ftxui::yframe,
                                      ftxui::separator(),
                                    }) |
                                    ftxui::border;
                           });
  renderer |= ftxui::CatchEvent(
    [&](ftxui::Event event)
    {
      if (event == ftxui::Event::Character('q'))
      {
        screen.ExitLoopClosure()();
        return true;
      }
      return false;
    });

  ftxui::Loop loop(&screen, renderer);
  while (!loop.HasQuitted())
  {
    if (!Mix_PlayingMusic()) { break; }
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
