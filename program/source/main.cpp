#define SDL_MAIN_HANDLED

#include <cctype>
#include <cmath>
#include <cstdlib>

#include "application.hpp"
#include "interface.hpp"
#include "main.hpp"
#include "system.hpp"

int main()
{
  tuim::system::initialize_sdl();
  tuim::application::initialize_playlists();
  tuim::interface::initialize_menus();
  tuim::interface::initialize_containers();
  tuim::interface::initialize_renderer();

  tuim::application::run_loop();

  tuim::system::cleanup();
  return EXIT_SUCCESS;
}
