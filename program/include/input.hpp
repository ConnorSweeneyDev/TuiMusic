#pragma once

#include <string>

namespace tuim::input
{
  bool menu_down(int amount, bool is_song);
  bool menu_up(int amount, bool is_song);
  bool menu_open_or_close(bool should_open);
  bool menu_select(bool is_song);
  bool shuffle_current_playlist(bool is_song);
  bool toggle_search();
  bool cancel_search();
  bool append_to_search_query(std::string character);
  bool pop_from_search_query();

  bool pause_or_play();
  bool seek_forward(int percentage);
  bool seek_backward(int percentage);
  bool seek_to(int percentage);
  bool volume_up(int amount);
  bool volume_down(int amount);
  bool end_song();
  bool quit();
}
