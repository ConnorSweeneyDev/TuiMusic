#define SDL_MAIN_HANDLED

#include <cctype>
#include <cmath>
#include <cstdlib>

#include "application.hpp"
#include "interface.hpp"
#include "main.hpp"
#include "system.hpp"

int main(int argc, char *argv[])
{
  tuim::system::initialize_sdl();
  tuim::application::initialize_playlists(argc, argv);
  tuim::application::initialize_state();
  tuim::interface::initialize_menus();
  tuim::interface::initialize_containers();
  tuim::interface::initialize_renderer();

  tuim::application::run_loop();

  tuim::system::cleanup();
  return EXIT_SUCCESS;
}
