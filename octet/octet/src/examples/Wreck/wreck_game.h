////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene using bullet for physics effects.
  class wreck_game : public app {

    race_track race_track;
    vehicle vehicle_instance;

    // scene for drawing box
    ref<visual_scene> app_scene;
    btDefaultCollisionConfiguration config; /// setup for the world
    btCollisionDispatcher *dispatcher; /// handler for collisions between objects
    btDbvtBroadphase *broadphase; /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver *solver; /// handler to resolve collisions
    btDiscreteDynamicsWorld *world; /// physics world, contains rigid bodies

    dynarray<btRigidBody*> rigid_bodies;
    dynarray<scene_node*> nodes;

    vec3 camAngle = (0.0f, 0.0f, 0.0f);

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
      //keyboard inputs - car movement with keyboard and xbox controller
      vehicle_instance.update();

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      world->stepSimulation(1.0f / 30, 1, 1.0f / 30);

      //update the rigid bodies
      btCollisionObjectArray &array = world->getCollisionObjectArray();
      for (int i = 0; i != array.size(); ++i) {
        btCollisionObject *co = array[i];
        scene_node *vehicle_nodes = (scene_node *)co->getUserPointer();
        if (vehicle_nodes) {
          if (i == 7){ //the chassis
            scene_node *cameraNode = app_scene->get_camera_instance(0)->get_node();
            vehicle_nodes->add_child(cameraNode);
            mat4t &cameraMatrix = cameraNode->access_nodeToParent();
            cameraNode->loadIdentity();
            cameraMatrix.translate(-30, 14, 0);
            //if (xbox_controller.refresh()){
            //camera_angles.x() = xbox_controller.right_analog_x;
            //camera_angles.y() = xbox_controller.right_analog_y;
            //}
            //default positions - facing camera
            cameraMatrix.rotateY(270.0f);
            cameraMatrix.rotateX(-20);
            if (is_key_down('X')){
              cameraMatrix.rotateY(camAngle.x());
              cameraMatrix.rotateX(camAngle.y() - 30);
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
