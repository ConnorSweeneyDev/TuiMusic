#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "D:/CPP/TuiMusic/external/include/ftxui/screen/screen.hpp"
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
#include "ftxui/util/ref.hpp"
#include "id3v2lib.compat.h"

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

  ID3v2_tag *song_tag = load_tag(song.string().c_str());
  if (song_tag == nullptr)
  {
    std::cout << "load_tag Error: \"" << song << "\": Could not read tags." << std::endl;
    exit(EXIT_FAILURE);
  }
  ID3v2_frame *title_frame = tag_get_title(song_tag);
  ID3v2_frame_text_content *title_content = parse_text_frame_content(title_frame);
  std::string title = title_content->data;
  ID3v2_frame *artist_frame = tag_get_artist(song_tag);
  ID3v2_frame_text_content *artist_content = parse_text_frame_content(artist_frame);
  std::string artist = artist_content->data;

  auto screen = ftxui::ScreenInteractive::Fullscreen();
  ftxui::Screen::Cursor cursor;
  cursor.shape = ftxui::Screen::Cursor::Hidden;
  screen.SetCursor(cursor);

  std::vector<std::string> entries = {title + " - " + artist, "test"};
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
                                      song_list->Render(),
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
