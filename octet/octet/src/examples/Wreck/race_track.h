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
      track->setFriction(10);
      the_world->addRigidBody(track);
      track->setUserPointer(track_nodes);

    }


    void init(app *app, visual_scene *app_scene, btDiscreteDynamicsWorld *world){
      this->the_app = app;
      this->app_scene = app_scene;
      this->the_world = world;

      mat4t modelToWorld;

      //create our texture here for the road
      
      create_track_component(modelToWorld, new mesh_box(vec3(400.0f, 0.5f, 400.0f)), new material(new image("assets/floor.jpg")));
      material *track_mat = new material(new image("assets/road_texture.jpg"));
      modelToWorld.translate(0.0f, 2.0f, 0.0f);
      create_track_component(modelToWorld, new mesh_box(vec3(50.0f, 0.5f, 10.0f)), track_mat);
      modelToWorld.rotateY90();
      modelToWorld.translate(60.0f, 0.0f, 60.0f);
      create_track_component(modelToWorld, new mesh_box(vec3(70.0f, 0.5f, 10.0f)), track_mat);
      modelToWorld.translate(0.0f, 0.0f, 20.0f);
      create_track_component(modelToWorld, new mesh_box(vec3(70.0f, 0.5f, 10.0f)), track_mat);
      modelToWorld.rotateX(-10);
      modelToWorld.translate(65.0f, 0.0f, 0.0f);
      create_track_component(modelToWorld, new mesh_box(vec3(20.0f, 0.5f, 10.0f)), track_mat);
      //deadend on the left hand side to the ramp
      mat4t deadend = modelToWorld;
      deadend.rotateX(-80);
      deadend.translate(40.0, 20.0f, 0.0f);
      material *deadend_mat = new material(new image("assets/deadend.jpg"));
      create_track_component(deadend, new mesh_box(vec3(5.0f, 50.0f, 10.0f)), deadend_mat);
      modelToWorld.rotateX(10);
      modelToWorld.translate(59.5f, 3.45f, 0.0f);
      create_track_component(modelToWorld, new mesh_box(vec3(40.0f, 0.5f, 10.0f)), track_mat);
      modelToWorld.rotateY(-90);
      modelToWorld.translate
    }

    void update(){

      
    }

    ~race_track() {
    }
  };
}