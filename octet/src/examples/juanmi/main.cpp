
/// @file main.cpp
/// @Author Juanmi Huertas Delgado (juanmihd)
/// @date July, 2014
/// @brief This is the main.cpp for the Panchito Jog project
///
/// In this file you can find the main call to the app.
/// This project started only as an idea to try some of the features of 
/// the OCTET Framework
///

//This calls and initializes the BULLET library
#define OCTET_BULLET 1
//I will use the octet library
#include "../../octet.h"
//The rest of the code is in PanchitoJog.h
#include "PanchitoJog.h"

/// This will run the PanchitoJog demo
int main(int argc, char **argv) {

  // set up the platform.
  octet::app::init_all(argc, argv);

  // our application.
  octet::PanchitoJog app(argc, argv);
  app.init();

  // open windows
  octet::app::run_all_apps();
}


