#pragma once

#include <string>

#include "application.hpp"

namespace tuim::utility
{
  std::string seconds_to_minutes(int seconds);
  float get_decibels(application::Song &song);
}
