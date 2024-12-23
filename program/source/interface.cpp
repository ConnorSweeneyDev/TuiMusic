#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/color.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/util/ref.hpp"

#include "application.hpp"
#include "input.hpp"
#include "interface.hpp"

namespace tuim::interface
{
  ftxui::Component ReactiveMenu(std::vector<std::string> *entries, int *selected)
  {
    auto option = ftxui::MenuOption::Vertical();
    option.focused_entry = ftxui::Ref<int>(selected);
    option.entries_option.transform = [](ftxui::EntryState state)
    {
      std::string icon = application::paused ? "⏸︎ " : "⏵︎ ";
      state.label = (state.active ? icon : "  ") + state.label;
      ftxui::Element element = ftxui::text(state.label);
      if (state.focused) element = element | ftxui::bgcolor(ftxui::Color::RGBA(0, 0, 0, 0));
      if (state.active)
        element = element | (application::paused ? ftxui::color(ftxui::Color::Red) : ftxui::color(ftxui::Color::Blue)) |
                  ftxui::bold;
      return element;
    };
    return Menu(entries, selected, option);
  }

  void initialize_menus()
  {
    ftxui::Screen::Cursor cursor;
    cursor.shape = ftxui::Screen::Cursor::Hidden;
    screen.SetCursor(cursor);

    for (auto &playlist : application::playlists)
      if ((int)playlist->name.length() > playlist_menu_max_width)
        playlist_menu_max_width = (int)playlist->name.length();
    playlist_menu_max_width += 3;
    for (auto &playlist : application::playlists) playlist_menu_entries.push_back(playlist->name);
    playlist_menu = ReactiveMenu(&playlist_menu_entries, &hovered_playlist);
    playlist_menu |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (event == ftxui::Event::j) return input::menu_down(1, false);
        if (event == ftxui::Event::k) return input::menu_up(1, false);
        if (event == ftxui::Event::J) return input::menu_down(12, false);
        if (event == ftxui::Event::K) return input::menu_up(12, false);
        if (event == ftxui::Event::B) return input::menu_down(100000, false);
        if (event == ftxui::Event::T) return input::menu_up(100000, false);
        if (event == ftxui::Event::l) return input::menu_open_or_close(false);
        if (event == ftxui::Event::s) return input::shuffle_current_playlist(false);
        if (event == ftxui::Event::Return) return input::menu_select(false);
        return false;
      });

    for (auto &song : application::playlists[(size_t)application::current_playlist_index]->songs)
      song_menu_entries.push_back(song.title + " ┃ " + song.artist);
    song_menu = ReactiveMenu(&song_menu_entries, &hovered_song);
    song_menu |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (event == ftxui::Event::j) return input::menu_down(1, true);
        if (event == ftxui::Event::k) return input::menu_up(1, true);
        if (event == ftxui::Event::J) return input::menu_down(12, true);
        if (event == ftxui::Event::K) return input::menu_up(12, true);
        if (event == ftxui::Event::B) return input::menu_down(100000, true);
        if (event == ftxui::Event::T) return input::menu_up(100000, true);
        if (event == ftxui::Event::h) return input::menu_open_or_close(true);
        if (event == ftxui::Event::s) return input::shuffle_current_playlist(true);
        if (event == ftxui::Event::Return) return input::menu_select(true);
        return false;
      });
  }

  void initialize_containers()
  {
    container =
      ftxui::ResizableSplitLeft(playlist_menu | ftxui::yframe, song_menu | ftxui::yframe, &playlist_menu_width);
    container |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (event == ftxui::Event::p) return input::pause_or_play();
        if (event == ftxui::Event::L) return input::seek_forward(5);
        if (event == ftxui::Event::H) return input::seek_backward(5);
        if (event == ftxui::Event::u) return input::volume_up(1);
        if (event == ftxui::Event::d) return input::volume_down(1);
        if (event == ftxui::Event::U) return input::volume_up(5);
        if (event == ftxui::Event::D) return input::volume_down(5);
        if (event == ftxui::Event::n) return input::end_song();
        if (event == ftxui::Event::Escape) return input::escape();
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
                 ftxui::hbox({ftxui::text(application::get_information_bar())}) | ftxui::center | ftxui::bold |
                   (application::paused ? ftxui::color(ftxui::Color::Red) : ftxui::color(ftxui::Color::Blue)),
                 ftxui::hbox({ftxui::text(application::get_progress_in_minutes()),
                              ftxui::gaugeRight(application::get_progress_as_percentage()),
                              ftxui::text(application::get_duration_in_minutes()),
                              ftxui::text(application::get_formatted_volume())}) |
                   ftxui::bold |
                   (application::paused ? ftxui::color(ftxui::Color::Red) : ftxui::color(ftxui::Color::Blue)),
                 ftxui::separator(),
                 container->Render(),
               }) |
               ftxui::borderEmpty;
      });
    song_menu->TakeFocus();
  }
}
