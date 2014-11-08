////////////////////////////////////////////////////////////////////////////////
//
// Ryan Singh 2014
//
// 
// 

#define OCTET_BULLET 1

#include "../../octet.h"
#include "xbox_controller.h"
#include "vehicle.h"
#include "race_track.h"
#include "wreck_game.h"


/// Create a box with octet
int main(int argc, char **argv) {
  // set up the platform.
  octet::app::init_all(argc, argv);
  // our application.
  octet::wreck_game app(argc, argv);
  app.init();
  // open windows
  octet::app::run_all_apps();
}
