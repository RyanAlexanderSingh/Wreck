////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {

  ///Class to create a create_shape using hinges
  class create_shape : public resource {

    app *the_app;
    visual_scene *app_scene;
    btDiscreteDynamicsWorld *the_world;

  private:
    mat4t modelToWorld;
    dynarray<btRigidBody*> create_shapes;


  public:
    create_shape()
    {
    }

    //this is a simple version of the function - creating a shape with no mass, useful for race tracks
    void shape_generator(mat4t_in modelToWorld, mesh *msh, material *mtl){
      
      scene_node *node = new scene_node();
      node->access_nodeToParent() = modelToWorld;
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, msh, mtl));

      btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
      btVector3 pos(get_btVector3(modelToWorld[3].xyz()));
      btCollisionShape *shape = msh->get_bullet_shape();

      btTransform transform(matrix, pos);
      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btVector3 inertiaTensor;
      shape->calculateLocalInertia(0.0f, inertiaTensor);
      btRigidBody *track = new btRigidBody(0.0f, motionState, shape, inertiaTensor);
      the_world->addRigidBody(track);
      track->setUserPointer(node);
    }

    void shape_generator(mat4t_in axilsize, mesh *msh, material *mtl, dynarray <btRigidBody*> *rbArray, bool is_dynamic, btScalar mass){

      scene_node *vehicle_nodes = new scene_node();
      vehicle_nodes->access_nodeToParent() = axilsize;
      app_scene->add_child(vehicle_nodes);
      app_scene->add_mesh_instance(new mesh_instance(vehicle_nodes, msh, mtl));
      btMatrix3x3 matrix(get_btMatrix3x3(axilsize));
      btVector3 pos(get_btVector3(axilsize[3].xyz()));
      btCollisionShape *shape = msh->get_bullet_shape();

      btTransform transform(matrix, pos);
      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btVector3 inertiaTensor;
      shape->calculateLocalInertia(mass, inertiaTensor);
      btRigidBody *component = new btRigidBody(mass, motionState, shape, inertiaTensor);
      the_world->addRigidBody(component);
      component->setUserPointer(vehicle_nodes);
      rbArray->push_back(component);

      }

    void init(app *app, visual_scene *app_scene, btDiscreteDynamicsWorld *world){
      this->the_app = app;
      this->app_scene = app_scene;
      this->the_world = world;  

      mat4t modelToWorld;
      modelToWorld.translate(0, 2, 3);
      //create our texture here for the road
      material *track_mat = new material(vec4(0, 0, 0, 1));
      shape_generator(modelToWorld, new mesh_box(vec3(200.0f, 0.5f, 200.0f)), track_mat);

    }

    void update(vec3 camera_angles){
      //update the car
      btCollisionObjectArray &array = the_world->getCollisionObjectArray();
      for (int i = 0; i != array.size(); ++i) {
        btCollisionObject *co = array[i];
        scene_node *vehicle_nodes = (scene_node *)co->getUserPointer();
        if (vehicle_nodes) {
          if (i == 1){ //the chassis
            scene_node *cameraNode = app_scene->get_camera_instance(0)->get_node();
            vehicle_nodes->add_child(cameraNode);
            mat4t &cameraMatrix = cameraNode->access_nodeToParent();
            cameraNode->loadIdentity();
            cameraMatrix.translate(-20, 10, 0);
            //if (xbox_controller.refresh()){
            //camera_angles.x() = xbox_controller.right_analog_x;
            //camera_angles.y() = xbox_controller.right_analog_y;
            //}
            cameraMatrix.rotateY(camera_angles.x());
            cameraMatrix.rotateX(camera_angles.y() - 30);

          }
          mat4t &mat = vehicle_nodes->access_nodeToParent();
          co->getWorldTransform().getOpenGLMatrix(mat.get());
        }
      }
    }

    ~create_shape() {
    }
  };
}
