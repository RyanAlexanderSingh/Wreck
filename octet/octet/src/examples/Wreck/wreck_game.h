////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene using bullet for physics effects.
  class wreck_game : public app {

    vehicle vehicle_instance;
    camera camera_instance;

    // scene for drawing box
    ref<visual_scene> app_scene;
    btDefaultCollisionConfiguration config; /// setup for the world
    btCollisionDispatcher *dispatcher; /// handler for collisions between objects
    btDbvtBroadphase *broadphase; /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver *solver; /// handler to resolve collisions
    btDiscreteDynamicsWorld *world; /// physics world, contains rigid bodies

    dynarray<btRigidBody*> rigid_bodies;
    dynarray<scene_node*> nodes;

    
    float axil_direction_limit = 0.0f;
    float motor_velocity = 0.0f;

    void add_box(mat4t_in modelToWorld, vec3_in size, material *mat, btScalar rbMass) {

      btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
      btVector3 pos(get_btVector3(modelToWorld[3].xyz()));
      btCollisionShape *shape = new btBoxShape(get_btVector3(size));
      btTransform transform(matrix, pos);
      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btScalar mass = rbMass;
      btVector3 inertiaTensor;
      shape->calculateLocalInertia(mass, inertiaTensor);

      btRigidBody * rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);
      rigid_body->setFriction(1);
      world->addRigidBody(rigid_body);
      rigid_bodies.push_back(rigid_body);

      mesh_box *box = new mesh_box(size);
      scene_node *node = new scene_node(modelToWorld, atom_sid);
      nodes.push_back(node);
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, mat));
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
      app_scene->get_camera_instance(0)->set_far_plane(20000);
      app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(0.0f, 3.0f, 20.0f);

      camera_instance.init(this);
      vehicle_instance.init(this, *&app_scene, *&world);
      
      
      mat4t modelToWorld;
      material *floor_mat = new material(new image("assets/floor.jpg"));

      // add the ground (as a static object)
      add_box(modelToWorld, vec3(200.0f, 0.5f, 200.0f), floor_mat, 0.0f);
    }


    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

      //camera_instance.update();
      vehicle_instance.update();

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);
      world->stepSimulation(1.0f / 30, 1, 1.0f / 30);

      //update the rigid bodies
      for (unsigned i = 0; i != rigid_bodies.size(); ++i) {
        btRigidBody *rigid_body = rigid_bodies[i];
        btQuaternion btq = rigid_body->getOrientation();
        btVector3 pos = rigid_body->getCenterOfMassPosition();
        quat q(btq[0], btq[1], btq[2], btq[3]);
        //forming the modelToWorld matrix
        mat4t modelToWorld;
        modelToWorld = q;
        modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);//position
        nodes[i]->access_nodeToParent() = modelToWorld;//apply to the node
      }

      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);
      // draw the scene
      app_scene->render((float)vx / vy);
    }

  };
}
