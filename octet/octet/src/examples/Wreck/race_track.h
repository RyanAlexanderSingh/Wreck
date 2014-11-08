////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

namespace octet {

  ///Class to create a race_track using hinges
  class race_track : public resource {

    app *the_app;
    visual_scene *app_scene;
    btDiscreteDynamicsWorld *the_world;

  public:
    race_track()
    {
    }

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
      
      dynarray<unsigned char> file;
      app_utils::get_url(file, "assets/race_track.txt");
      dynarray<char> track_variable;
      dynarray<float> track_specifications;
      material *track_mat = new material(new image("assets/road_texture.jpg"));
      //create the roads & barriers
      for (unsigned i = 0; i != file.size(); ++i){
        unsigned c = file[i];
        //33 in UTF-8 represents an exclaimation mark, our breakpoint for reading the whole file
        if (c != 33){
          //commas are used to separate the values, 10 and 13 represent LF and CR
          if (c != 44 && c != 10 && c != 13){
            track_variable.push_back(c); //if no comma, it's part of the same variable
          }
          //94 represents change material to barrier(^)
          if (c == 94){
            modelToWorld.loadIdentity();
            track_mat = new material(new image("assets/deadend.jpg"));
          }
          if (c == 44){
            track_variable.push_back('\0'); //null terminate the buffer
            //parse char string to double and assign it to a float
            float value = strtod(track_variable.data(), NULL);
            track_specifications.push_back(value);
            track_variable.reset();
          }
          //put all the stored information in as a newline approaches
          if (c == 10){
            track_specifications.push_back('\0'); //null terminate the buffer 
            modelToWorld.rotate(track_specifications[0], track_specifications[1], track_specifications[2], track_specifications[3]);
            modelToWorld.translate(track_specifications[4], track_specifications[5], track_specifications[6]);
            create_track_component(modelToWorld, new mesh_box(vec3(track_specifications[7], track_specifications[8], track_specifications[9])), track_mat, true);
            track_specifications.reset();
          }
        }
      }
    }

    ~race_track() {
    }
  };
}