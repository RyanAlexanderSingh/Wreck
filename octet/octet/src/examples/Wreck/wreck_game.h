////////////////////////////////////////////////////////////////////////////////
//
// Ryan Singh 2014
//
// wreck_game.h - main header file for init the race track and vehicle classes
//
namespace octet {
  ///Scene using bullet physics to create a vehicle without using raycast vehicle.
  ///The scene produces a vehicle from rigid bodies and attached using hinge constraints: vehicle.h.
  ///A sample race_track is provided which is read in from a text file: race_track.h.
  ///The inputs are default to keyboard but an xbox controller can be read (better control on xbox controller): xbox_controller.h.
  class wreck_game : public app {

    race_track race_track;
    vehicle vehicle_instance;
    xbox_controller xbox_controller;

    // scene for drawing box
    ref<visual_scene> app_scene;
    btDefaultCollisionConfiguration config; /// setup for the world
    btCollisionDispatcher *dispatcher; /// handler for collisions between objects
    btDbvtBroadphase *broadphase; /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver *solver; /// handler to resolve collisions
    btDiscreteDynamicsWorld *world; /// physics world, contains rigid bodies

    vec2 camAngle = (0.0f, 0.0f); //vec2 to store x and y pos of camera angle.

    ///this function is responsible for moving the camera based on mouse position
    void move_camera(int x, int y, HWND *w)
    {
      static bool is_mouse_moving = true;

      if (is_mouse_moving){
        int vx, vy;
        get_viewport_size(vx, vy);
        float dx = x - vx * 0.5f;
        float dy = y - vy * 0.5f;

        //apply the deltaX and deltaY of the mouse to the camera angles.
        const float sensitivity = -0.5f;
        camAngle.x() += dx * sensitivity;
        camAngle.y() += dy * sensitivity;
        is_mouse_moving = false;

        //set the position of the mouse to the center of the window
        tagPOINT p;
        p.x = vx * 0.5f;
        p.y = vy * 0.5f;
        ClientToScreen(*w, &p);
        SetCursorPos(p.x, p.y);
      }
      else
      {
        is_mouse_moving = true;
      }
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    wreck_game(int argc, char **argv) : app(argc, argv){
      dispatcher = new btCollisionDispatcher(&config);
      broadphase = new btDbvtBroadphase();
      solver = new btSequentialImpulseConstraintSolver();
      world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
    }
    ~wreck_game() {
      delete world;
      delete solver;
      delete broadphase;
      delete dispatcher;
    }

    /// this is called once OpenGL is initialized
    void app_init() {

      //hide the cursor
      ShowCursor(false);

      //set gravity for the world
      world->setGravity(btVector3(0.0f, -15.0f, 0.0f));

      //load the scene and camera
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_near_plane(1);
      app_scene->get_camera_instance(0)->set_far_plane(2000);
      app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(0.0f, 3.0f, 20.0f);

      //create the race track
      race_track.init(this, *&app_scene, *&world);

      //create the car
      vehicle_instance.init(this, *&app_scene, *&world);
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      //keyboard inputs - car movement with keyboard and xbox controller
      vehicle_instance.update(vx, vy);

      world->stepSimulation(1.0f / 30, 1, 1.0f / 30);

      //update the rigid bodies
      btCollisionObjectArray &array = world->getCollisionObjectArray();
      for (int i = 0; i != array.size(); ++i) {
        btCollisionObject *co = array[i];
        scene_node *vehicle_nodes = (scene_node *)co->getUserPointer();
        if (vehicle_nodes) {
          if (i == 49){ //the chassis - shouldn't programatically assign camera
            scene_node *cameraNode = app_scene->get_camera_instance(0)->get_node();
            vehicle_nodes->add_child(cameraNode);
            mat4t &cameraMatrix = cameraNode->access_nodeToParent();
            cameraNode->loadIdentity();
            cameraMatrix.translate(-30, 14, 0);
            if (is_key_down('X')){ //allow a free rotating camera 
              cameraMatrix.rotateY(camAngle.x());
              cameraMatrix.rotateX(camAngle.y() - 30);
            }
            //default positions - static camera facing vehicle
            else{
              cameraMatrix.rotateY(270.0f);
              cameraMatrix.rotateX(-20);
            }
          }
          mat4t &mat = vehicle_nodes->access_nodeToParent();
          co->getWorldTransform().getOpenGLMatrix(mat.get());
        }
      }
      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);
      // draw the scene
      app_scene->render((float)vx / vy);
    }

  };
}
