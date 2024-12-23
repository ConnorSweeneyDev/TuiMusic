#include <cstdlib>
#include <iostream>
#include <memory>
#include <regex>
#include <stdio.h>
#include <string>

#include "application.hpp"
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

  float get_decibels(application::Song &song)
  {
    std::string command = "ffmpeg -i \"" + song.path.string() + "\" -filter:a volumedetect -f null /dev/null 2>&1";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe)
    {
      std::cout << "Failed to open pipe!" << std::endl;
      exit(EXIT_FAILURE);
    }
    float decibels = -14.0f;
    std::string ffmpeg_output;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) ffmpeg_output += buffer;
    pipe.reset();
    std::regex ffmpeg_regex("mean_volume: -?[0-9]+.[0-9]+");
    std::smatch ffmpeg_match;
    if (std::regex_search(ffmpeg_output, ffmpeg_match, ffmpeg_regex))
    {
      ffmpeg_output = ffmpeg_match[0];
      std::regex decibels_regex("-?[0-9]+.[0-9]+");
      std::smatch decibels_match;
      if (std::regex_search(ffmpeg_output, decibels_match, decibels_regex))
        decibels = std::stof(decibels_match[0]);
      else
        decibels = -14.0f;
    }
    else
      decibels = -14.0f;
    return decibels;
  }
}
