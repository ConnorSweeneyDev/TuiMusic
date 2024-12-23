#include <string>

#include "utility.hpp"

namespace tuim::utility
{
  std::string seconds_to_minutes(int seconds)
  {
    int minutes = seconds / 60;
    int seconds_remaining = seconds % 60;
    return std::to_string(minutes) + ":" +
           ((seconds_remaining < 10) ? "0" + std::to_string(seconds_remaining) : std::to_string(seconds_remaining));
  }
}
