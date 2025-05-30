#include <component/event.hpp>
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
#include "utility.hpp"

namespace tuim::interface
{
  ftxui::Decorator ReactiveColor()
  {
    return [&](ftxui::Element element)
    {
      return element | (application::searching ? ftxui::color(ftxui::Color::Yellow)
                        : application::paused  ? ftxui::color(ftxui::Color::Red)
                                               : ftxui::color(ftxui::Color::Blue));
    };
  }

  ftxui::Component PlaylistMenu(std::vector<std::string> *entries, int *selected)
  {
    auto option = ftxui::MenuOption::Vertical();
    option.focused_entry = ftxui::Ref<int>(selected);
    option.entries_option.transform = [&](ftxui::EntryState state)
    {
      size_t total = application::playlists.size();
      size_t total_length = std::to_string(total).length();

      int index = state.index + 1;
      size_t index_length = std::to_string(index).length();
      std::string index_padding = std::string(total_length - index_length, ' ');
      std::string index_formatted = index_padding + std::to_string(index) + " ";

      std::string icon_padding = std::string(total_length - 1, ' ');
      std::string icon = icon_padding + (application::paused ? "⏸︎ " : "⏵︎ ");

      int current_song_playlist_index = 0;
      for (auto &playlist : application::playlists)
        if (playlist == application::current_song_playlist)
          break;
        else
          current_song_playlist_index++;
      bool active = state.index == current_song_playlist_index;
      state.label = (active ? icon : index_formatted) + state.label;
      ftxui::Element element = ftxui::text(state.label);
      if (state.focused) element = element | ftxui::bgcolor(ftxui::Color::RGBA(0, 0, 0, 0));
      if (state.active) element = element | ftxui::bold | ReactiveColor();
      return element;
    };
    return Menu(entries, selected, option);
  }

  ftxui::Component SongMenu(std::vector<std::string> *entries, int *selected)
  {
    auto option = ftxui::MenuOption::Vertical();
    option.focused_entry = ftxui::Ref<int>(selected);
    option.entries_option.transform = [&](ftxui::EntryState state)
    {
      size_t total = application::playlists[(size_t)application::current_playlist_index]->songs.size();
      size_t total_length = std::to_string(total).length();

      int index = state.index + 1;
      size_t index_length = std::to_string(index).length();
      std::string index_padding = (playlist_menu->Focused() ? " " : "") + std::string(total_length - index_length, ' ');
      std::string index_formatted = index_padding + std::to_string(index) + " ";

      std::string icon_padding = (playlist_menu->Focused() ? " " : "") + std::string(total_length - 1, ' ');
      std::string icon = icon_padding + (application::paused ? "⏸︎ " : "⏵︎ ");

      bool active =
        state.index == application::current_song_index &&
        application::playlists[(size_t)application::current_playlist_index] == application::current_song_playlist;
      state.label = (active ? icon : index_formatted) + state.label;
      ftxui::Element element = ftxui::text(state.label);
      if (state.focused) element = element | ftxui::bgcolor(ftxui::Color::RGBA(0, 0, 0, 0));
      if (state.active) element = element | ftxui::bold | ReactiveColor();
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
      if (playlist->name.length() + std::to_string(playlist->songs.size()).length() > (size_t)playlist_menu_max_width)
        playlist_menu_max_width = (int)playlist->name.length() + (int)std::to_string(playlist->songs.size()).length();
    playlist_menu_max_width += ((int)std::to_string(application::playlists.size()).length() - 1) + 8;
    for (auto &playlist : application::playlists)
      playlist_menu_entries.push_back("┃ " + playlist->name + " ┃ " + std::to_string(playlist->songs.size()));
    playlist_menu = PlaylistMenu(&playlist_menu_entries, &hovered_playlist);
    playlist_menu |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (event == ftxui::Event::j || event == ftxui::Event::ArrowDown) return input::menu_down(1, false);
        if (event == ftxui::Event::k || event == ftxui::Event::ArrowUp) return input::menu_up(1, false);
        if (event == ftxui::Event::J) return input::menu_down(12, false);
        if (event == ftxui::Event::K) return input::menu_up(12, false);
        if (event == ftxui::Event::B) return input::menu_down(100000, false);
        if (event == ftxui::Event::T) return input::menu_up(100000, false);
        if (event == ftxui::Event::l || event == ftxui::Event::ArrowRight) return input::menu_open_or_close(false);
        if (event == ftxui::Event::s) return input::shuffle_current_playlist(false);
        if (event == ftxui::Event::Return) return input::menu_select(false);

        if (event == ftxui::Event::p) return input::pause_or_play();
        if (event == ftxui::Event::L) return input::seek_forward(5);
        if (event == ftxui::Event::H) return input::seek_backward(5);
        if (utility::is_number(event.character())) return input::seek_to(std::stoi(event.character()) * 10);
        if (event == ftxui::Event::u) return input::volume_up(1);
        if (event == ftxui::Event::d) return input::volume_down(1);
        if (event == ftxui::Event::U) return input::volume_up(5);
        if (event == ftxui::Event::D) return input::volume_down(5);
        if (event == ftxui::Event::n) return input::end_song();

        if (event == ftxui::Event::Escape) return input::quit();

        return false;
      });

    for (auto &song : application::playlists[(size_t)application::current_playlist_index]->songs)
    {
      if (song.artist.empty())
      {
        song_menu_entries.push_back("┃ " + song.title);
        continue;
      }

      song_menu_entries.push_back("┃ " + song.artist + " ┃ " + song.title);
    }
    song_menu = SongMenu(&song_menu_entries, &hovered_song);
    song_menu |= ftxui::CatchEvent(
      [&](ftxui::Event event)
      {
        if (application::searching)
        {
          if (event.is_character()) return input::append_to_search_query(event.character());
          if (event == ftxui::Event::Backspace) return input::pop_from_search_query();
          if (event == ftxui::Event::CtrlW) return input::pop_word_from_search_query();
          if (event == ftxui::Event::Return)
          {
            input::menu_select(true);
            return input::toggle_search();
          }
          if (event == ftxui::Event::Escape) return input::cancel_search();
          if (event == ftxui::Event::ArrowLeft || event == ftxui::Event::ArrowRight) return true;
        }
        else
        {
          if (event == ftxui::Event::j || event == ftxui::Event::ArrowDown) return input::menu_down(1, true);
          if (event == ftxui::Event::k || event == ftxui::Event::ArrowUp) return input::menu_up(1, true);
          if (event == ftxui::Event::J) return input::menu_down(12, true);
          if (event == ftxui::Event::K) return input::menu_up(12, true);
          if (event == ftxui::Event::B) return input::menu_down(100000, true);
          if (event == ftxui::Event::T) return input::menu_up(100000, true);
          if (event == ftxui::Event::h || event == ftxui::Event::ArrowLeft) return input::menu_open_or_close(true);
          if (event == ftxui::Event::s) return input::shuffle_current_playlist(true);
          if (event == ftxui::Event::Return) return input::menu_select(true);

          if (event == ftxui::Event::p) return input::pause_or_play();
          if (event == ftxui::Event::L) return input::seek_forward(5);
          if (event == ftxui::Event::H) return input::seek_backward(5);
          if (utility::is_number(event.character())) return input::seek_to(std::stoi(event.character()) * 10);
          if (event == ftxui::Event::u) return input::volume_up(1);
          if (event == ftxui::Event::d) return input::volume_down(1);
          if (event == ftxui::Event::U) return input::volume_up(5);
          if (event == ftxui::Event::D) return input::volume_down(5);
          if (event == ftxui::Event::n) return input::end_song();

          if (event == ftxui::Event::Escape) return input::quit();
        }

        if (event == ftxui::Event::CtrlF) return input::toggle_search();
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
        if (event.mouse().motion == ftxui::Mouse::Motion::Moved) return true;
        if (event.mouse().motion == ftxui::Mouse::Motion::Pressed) return true;
        return false;
      });
  }

  void initialize_renderer()
  {
    renderer = ftxui::Renderer(container,
                               [&]
                               {
                                 return ftxui::vbox({
                                          ftxui::hbox({ftxui::text(application::get_information_bar())}) |
                                            ReactiveColor() | ftxui::bold | ftxui::center,
                                          ftxui::hbox({ftxui::text(application::get_progress_in_minutes()),
                                                       ftxui::gaugeRight(application::get_progress_as_percentage()),
                                                       ftxui::text(application::get_duration_in_minutes()),
                                                       ftxui::text(application::get_formatted_volume()),
                                                       ftxui::text(application::get_search_text())}) |
                                            ReactiveColor() | ftxui::bold,
                                          ftxui::separator(),
                                          container->Render(),
                                        }) |
                                        ftxui::borderEmpty;
                               });
    song_menu->TakeFocus();
  }
}
