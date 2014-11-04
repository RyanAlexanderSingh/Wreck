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

  private:

    dynarray<btRigidBody*> race_tracks;


  public:
    race_track()
    {
    }

    void create_track_component(mat4t_in track_size, mesh *msh, material *mtl){
      scene_node *track_nodes = new scene_node();
      track_nodes->access_nodeToParent() = track_size;
      app_scene->add_child(track_nodes);
      app_scene->add_mesh_instance(new mesh_instance(track_nodes, msh, mtl));

      btMatrix3x3 matrix(get_btMatrix3x3(track_size));
      btVector3 pos(get_btVector3(track_size[3].xyz()));
      btCollisionShape *shape = msh->get_bullet_shape();

      btTransform transform(matrix, pos);
      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btVector3 inertiaTensor;
      shape->calculateLocalInertia(0.0f, inertiaTensor);
      btRigidBody *track = new btRigidBody(0.0f, motionState, shape, inertiaTensor);
      the_world->addRigidBody(track);
      track->setUserPointer(track_nodes);
    }


    void init(app *app, visual_scene *app_scene, btDiscreteDynamicsWorld *world){
      this->the_app = app;
      this->app_scene = app_scene;
      this->the_world = world;

      mat4t modelToWorld;

      //create our texture here for the road
      material *track_mat = new material(vec4(0, 0, 0, 1));

      create_track_component(modelToWorld, new mesh_box(vec3(200.0f, 0.5f, 200.0f)), track_mat);
    }

    void update(){

      btCollisionObjectArray &array = the_world->getCollisionObjectArray();
      for (int i = 0; i != array.size(); ++i) {
        btCollisionObject *co = array[i];
        scene_node *track_nodes = (scene_node *)co->getUserPointer();
        if (track_nodes){
          mat4t &mat = track_nodes->access_nodeToParent();
          co->getWorldTransform().getOpenGLMatrix(mat.get());
        }
      }
    }

    ~race_track() {
    }
  };
}
