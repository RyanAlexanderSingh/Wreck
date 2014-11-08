////////////////////////////////////////////////////////////////////////////////
//
// Ryan Singh 
//
// race_track.h - class creates a race track with rigid bodies from reading in a .txt file
// 


namespace octet {

  ///Class for creating a race track with rigid bodies reading a .txt file.
  /** A race track is created by reading in a .txt file with 10 or 11 parameters.
  The first 4 parameters is the modelToWorld rotation, the next 3 is for the translation
  and the last 3 assign the size of the mesh box. If an 11th parameter then change the material to a road barrier.
  */
  class race_track : public resource {

    app *the_app;
    visual_scene *app_scene;
    btDiscreteDynamicsWorld *the_world;

  public:
    race_track()
    {
    }

    ///Creates a mesh and possibly a rigid body for the environment. 
    /**Mainly used to create the race track rigid mesh and rigid bodies but also used to create a skybox mesh.
    */
    void create_track_component(mat4t_in track_size, mesh *msh, material *mtl, bool is_rigid_body){

      scene_node *track_nodes = new scene_node();
      track_nodes->access_nodeToParent() = track_size;
      app_scene->add_child(track_nodes);
      app_scene->add_mesh_instance(new mesh_instance(track_nodes, msh, mtl));

      if (is_rigid_body){
        btMatrix3x3 matrix(get_btMatrix3x3(track_size));
        btVector3 pos(get_btVector3(track_size[3].xyz()));
        btCollisionShape *shape = msh->get_bullet_shape();

        btTransform transform(matrix, pos);
        btDefaultMotionState *motionState = new btDefaultMotionState(transform);
        btVector3 inertiaTensor;
        shape->calculateLocalInertia(0.0f, inertiaTensor);
        btRigidBody *track = new btRigidBody(0.0f, motionState, shape, inertiaTensor);
        track->setFriction(10);
        the_world->addRigidBody(track);
        track->setUserPointer(track_nodes);
      }
    }

    ///init the class, getting the app, app_scene and world from the main app (wreck_game.h)
    void init(app *app, visual_scene *app_scene, btDiscreteDynamicsWorld *world){
      this->the_app = app;
      this->app_scene = app_scene;
      this->the_world = world;

      //create the fake skybox
      mat4t skybox_m;
      material *skybox_mat = new material(new image("assets/seamless_sky.jpg"));
      create_track_component(skybox_m, new mesh_box(vec3(500.0f, 200.0f, 500.0f)), skybox_mat, false);

      mat4t modelToWorld;
      //create our texture here for the road
      create_track_component(modelToWorld, new mesh_box(vec3(400.0f, 0.5f, 400.0f)), new material(new image("assets/floor.jpg")), true);

      dynarray<unsigned char> file; //char array to store the contents of the file
      app_utils::get_url(file, "assets/race_track.txt");
      dynarray<char> track_variable; //stores individual track paramters from the file
      dynarray<float> track_parameters; //stores complete race track paramters - rotation, translation and size
      material *track_mat = new material(new image("assets/road_texture.jpg"));
      //create the roads & barriers
      for (unsigned i = 0; i != file.size(); ++i){
        unsigned c = file[i];
        //33 in UTF-8 represents an exclaimation mark, our breakpoint for reading the whole file
        if (c != 33){
          //commas are used to separate the values, 10 and 13 represent LF and CR
          if (c != 44 && c != 10 && c != 13){
            //if no comma, it's part of the same variable
            track_variable.push_back(c);
          }
          //94 represents change material to barrier(^)
          if (c == 94){
            modelToWorld.loadIdentity();
            track_mat = new material(new image("assets/deadend.jpg"));
          }
          if (c == 44){
            track_variable.push_back('\0'); //null terminate the buffer
            //parse char string to double and assign it to a float
            float value = strtod(track_variable.data(), NULL); //floating point representation of char array which stores individual parameters
            track_parameters.push_back(value);
            track_variable.reset();
          }
          //put all the stored information in as a newline approaches
          if (c == 10){
            track_parameters.push_back('\0'); //null terminate the buffer 
            modelToWorld.rotate(track_parameters[0], track_parameters[1], track_parameters[2], track_parameters[3]);
            modelToWorld.translate(track_parameters[4], track_parameters[5], track_parameters[6]);
            create_track_component(modelToWorld, new mesh_box(vec3(track_parameters[7], track_parameters[8], track_parameters[9])), track_mat, true);
            track_parameters.reset();
          }
        }
      }
    }

    ~race_track() {
    }
  };
}