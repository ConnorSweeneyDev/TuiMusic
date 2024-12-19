#include <cstdlib>
#include <iostream>
#include <string>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_main.h"
#include "SDL_mixer.h"
#include "component/component.hpp"
#include "component/component_options.hpp"
#include "dom/deprecated.hpp"
#include "dom/elements.hpp"
#include "dom/node.hpp"
#include "screen/color.hpp"
#include "screen/screen.hpp"

ftxui::ButtonOption Style()
{
  auto option = ftxui::ButtonOption::Animated();
  option.transform = [](const ftxui::EntryState &s)
  {
    auto element = ftxui::text(s.label);
    if (s.focused) { element |= ftxui::bold; }
    return element | ftxui::center | ftxui::borderEmpty | ftxui::flex;
  };
  return option;
}

int main(int argc, char *argv[])
{
  if (argc > 1 || argv[1] != nullptr)
  {
    std::cout << "No arguments are supported.\n";
    return EXIT_FAILURE;
  }

  if (SDL_Init(SDL_INIT_AUDIO) != 0)
  {
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
    exit(1);
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
  {
    std::cout << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
    exit(1);
  }
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Quit();

  auto summary = [&]
  {
    auto content = ftxui::vbox({
      ftxui::hbox({ftxui::text(L"- done:   "), ftxui::text(L"3") | ftxui::bold}) | color(ftxui::Color::Green),
      ftxui::hbox({ftxui::text(L"- active: "), ftxui::text(L"2") | ftxui::bold}) | color(ftxui::Color::RedLight),
      ftxui::hbox({ftxui::text(L"- queue:  "), ftxui::text(L"9") | ftxui::bold}) | color(ftxui::Color::Red),
    });
    return window(ftxui::text(L" Summary "), content);
  };

  auto document = ftxui::vbox({
    ftxui::hbox({
      summary(),
      summary(),
      summary() | ftxui::flex,
    }),
    summary(),
    summary(),
  });
  document = document | size(ftxui::WIDTH, ftxui::LESS_THAN, 80);

  auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(), ftxui::Dimension::Fit(document));
  Render(screen, document);

  std::cout << screen.ToString() << '\0' << std::endl;
  return EXIT_SUCCESS;
}
