///
/// @file PanchitoJog.h
/// @Author Juanmi Huertas Delgado (Juanmihd)
/// @date July, 2014
/// @brief In this file there is the PanchitoJog application
///
/// This file contains the core application, but requires the content of
/// PanchitoObject and Terrain. 
/// PanchitoObject contains the info regarding Panchito and the object
///	and terrain obtains the information of the ground
///

#include "PanchitoObject.h"
#include "terrain.h"

///
/// @brief This will be the actual game of PanchitoJog
///
namespace octet {
  /// Scene containing a box with octet.
  class PanchitoJog : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

	//behold, Panchito!
	PanchitoObject panchito;
	//terrain world;

  public:
    /// this is called when we construct the class before everything is initialised.
    PanchitoJog(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
	  

      panchito.init(-7,-7,-2,3);
	  panchito.LoadToScene(app_scene);
	  // We need to load the world BEFORE init the scene, as we will use the scene to init the blocks
	 // world.init(app_scene);
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene (this will draw Panchito, and the Score and other item of the GUI)
      app_scene->render((float)vx / vy);

	  // this will draw only the terrain. It's importante to do aside to be able to access to the 
	  // different blocks of terrain
	  //world.render();

	  // panchito is jogging, we have to animate him accordingly to that
	  panchito.animateJogging();
    }
  };
}
