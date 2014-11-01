////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene using bullet for physics effects.
  class wreck_game : public app {

    // scene for drawing box
    ref<visual_scene> app_scene;
    btDefaultCollisionConfiguration config; /// setup for the world
    btCollisionDispatcher *dispatcher; /// handler for collisions between objects
    btDbvtBroadphase *broadphase; /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver *solver; /// handler to resolve collisions
    btDiscreteDynamicsWorld *world; /// physics world, contains rigid bodies

    dynarray<btRigidBody*> rigid_bodies;
    dynarray<scene_node*> nodes;

    dynarray<btRigidBody*> wheels;
    dynarray<btRigidBody*> axils;

    //Axil-Wheel Hinges
    dynarray<btHingeConstraint*> hingeAW;

    //Chassis-Axil Hinges
    dynarray<btHingeConstraint*> hingeCA;

    vec3 camAngle = (0.0f, 0.0f, 0.0f);

    float axil_direction_limit = 0.0f;
    float target_angular_velocity = 0.0f;
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

    void create_car_component(mat4t_in axilsize, mesh *msh, material *mtl, dynarray <btRigidBody*> *rbArray, bool is_dynamic, btScalar mass){

      scene_node *node = new scene_node();
      node->access_nodeToParent() = axilsize;
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, msh, mtl));
      btMatrix3x3 matrix(get_btMatrix3x3(axilsize));
      btVector3 pos(get_btVector3(axilsize[3].xyz()));
      btCollisionShape *shape = msh->get_bullet_shape();

      btTransform transform(matrix, pos);
      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btVector3 inertiaTensor;
      shape->calculateLocalInertia(mass, inertiaTensor);
      btRigidBody *axil = new btRigidBody(mass, motionState, shape, inertiaTensor);
      world->addRigidBody(axil);
      axil->setUserPointer(node);
      rbArray->push_back(axil);

    }

    void create_hinges(btRigidBody *rbA, btRigidBody *rbB, dynarray <btHingeConstraint*> *hinge_array, vec3_in PivotA, vec3_in PivotB, vec3_in Axis, bool set_hinge_limits){

      btVector3 btPivotA = get_btVector3(PivotA);
      btVector3 btPivotB = get_btVector3(PivotB);
      btVector3 btAxis = get_btVector3(Axis);

      btHingeConstraint *hingeConstraint = new btHingeConstraint((*rbA), (*rbB), btPivotA, btPivotB, btAxis, btAxis);
      if (set_hinge_limits){
        hingeConstraint->setLimit(0.0f, 0.0f);
      }
      hinge_array->push_back(hingeConstraint);
      world->addConstraint(hingeConstraint, true);
    }

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
      app_scene->get_camera_instance(0)->set_far_plane(20000);
      app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(0.0f, 3.0f, 20.0f);

      mat4t modelToWorld;
      material *floor_mat = new material(new image("assets/floor.jpg"));

      // add the ground (as a static object)
      add_box(modelToWorld, vec3(200.0f, 0.5f, 200.0f), floor_mat, 0.0f);

      //add the car (a dynamic object)
      modelToWorld.loadIdentity();
      modelToWorld.rotate(90.0f, 0.0f, 1.0f, 0.0f);
      modelToWorld.translate(0.0f, 20.0f, 0.0f);
      vec3 chassis_size(3.0f, 0.125f, 2.0f);
      add_box(modelToWorld, chassis_size, floor_mat, 20.0f);
    
      vec3 axil_size(0.25f, 0.25f, 0.5f);
      material *wheel_mat = new material(new image("assets/tire.jpg"));
      material *red = new material(vec4(1, 0, 0, 1));
      for (int i = 0; i != 4; ++i){
        create_car_component(modelToWorld, new mesh_cylinder(zcylinder(vec3(0, 0, 0), 1.0f, 0.5f)), wheel_mat, &wheels, true, 10.0f);
        create_car_component(modelToWorld, new mesh_box(axil_size), red, &axils, true, 5.0f);
      }
    
      float dist_x = 1.2;
      float dist_y = 0.125f;
      float dist_z = chassis_size.z() - axil_size.z();

      //create the hinges for the chassis - axils
      create_hinges(*&rigid_bodies[1], *&axils[0], &hingeCA, vec3(dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[0], *&wheels[0], &hingeAW, vec3(0.0f, 0.0f, -0.575f), vec3(0.0f, 0.0f, 0.575f), vec3(0.0f, 0.0f, 1.0f), false);
      create_hinges(*&rigid_bodies[1], *&axils[1], &hingeCA, vec3(dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, -dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[1], *&wheels[1], &hingeAW, vec3(0.0f, 0.0f, 0.375f), vec3(0.0f, 0.0f, -0.375f), vec3(0.0f, 0.0f, 1.0f), false);
      create_hinges(*&rigid_bodies[1], *&axils[2], &hingeCA, vec3(-dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[2], *&wheels[2], &hingeAW, vec3(0.0f, 0.0f, -0.375f), vec3(0.0f, 0.0f, 0.375f), vec3(0.0f, 0.0f, 1.0f), false);
      create_hinges(*&rigid_bodies[1], *&axils[3], &hingeCA, vec3(-dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, -dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[3], *&wheels[3], &hingeAW, vec3(0.0f, 0.0f, 0.575f), vec3(0.0f, 0.0f, -0.575f), vec3(0.0f, 0.0f, 1.0f), false);

      // add the boxes (as dynamic objects)
      modelToWorld.translate(-4.5f, 10.0f, 0.0f);
      math::random rand;
    }
    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {


      keyboardInputs();
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
        if (i == 1)
        {
          scene_node *cameraNode = app_scene->get_camera_instance(0)->get_node();
          nodes[i]->add_child(cameraNode);
          mat4t &cameraMatrix = cameraNode->access_nodeToParent();
          cameraNode->loadIdentity();
          cameraMatrix.translate(-20, 10, 0);
          //cameraMatrix.rotateY(90);
          cameraMatrix.rotateY(camAngle.x());
          cameraMatrix.rotateX(camAngle.y() - 30);
        }
        modelToWorld = q;
        modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);//position
        nodes[i]->access_nodeToParent() = modelToWorld;//apply to the node
      }

      //update the wheels & axils
      btCollisionObjectArray &array = world->getCollisionObjectArray();
      for (int i = 0; i != array.size(); ++i) {
        btCollisionObject *co = array[i];
        scene_node *node = (scene_node *)co->getUserPointer();
        if (node) {
          mat4t &mat = node->access_nodeToParent();
          co->getWorldTransform().getOpenGLMatrix(mat.get());
        }
      }

      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);
      // draw the scene
      app_scene->render((float)vx / vy);
    }

    ///any random keyboard functions such as esc to close the game
    void keyboardInputs(){

      const float step_velocity = 0.2f;
      const float max_velocity = 10.0f;

      const float step_angle = 1.0f;
      const float max_angle = 7.0f;

      // rotation of the front two axils - turning the chassis left or right
      if (is_key_down('A') || is_key_down(key_left)) {
        if (axil_direction_limit > -(max_angle * 3.14159265f / 180.0f))
        {
          axil_direction_limit -= step_angle * 3.14159265f / 180.0f;
          rotate_axils(axil_direction_limit);
        }
      }
      if (is_key_down('D') || is_key_down(key_right)) {
        if (axil_direction_limit < (max_angle * 3.14159265f / 180.0f)){
          axil_direction_limit += step_angle * 3.14159265f / 180.0f;
          rotate_axils(axil_direction_limit);
        }
      }

      //moving the car forwards
      if (is_key_down('W') || is_key_down(key_up)){
        if (motor_velocity < max_velocity){
          motor_velocity += step_velocity;
          move_direction(motor_velocity, 10);
        }
      }
      //moving the car backwards - maximum velocity is less as we're moving backwards
      else if (is_key_down('S') || is_key_down(key_down)){
        if (motor_velocity > -max_velocity){
          motor_velocity -= step_velocity;
          move_direction(motor_velocity, 10);
        }
      }
      else if (motor_velocity != 0){
        if (motor_velocity > 0.0f){
          motor_velocity -= step_velocity * 2;
        }
        if (motor_velocity < 0.0f){
          motor_velocity += step_velocity * 2;
        }
        move_direction(motor_velocity, 10);
      }

      //close the progam
      if (is_key_down(key_esc))
      {
        exit(1);
      }
    }

    //used to control the car
    void move_direction(float motor_velocity, float max_motor_impulse){
      for (int i = 0; i != 4; ++i){
        axils[i]->activate(true);
        hingeAW[i]->enableAngularMotor(true, motor_velocity, max_motor_impulse);
      }
    }

    //rotate the front two axil's at an angle
    void rotate_axils(float axil_direction_limit){
      for (int i = 0; i != 2; ++i){
        axils[i]->activate(true);
        hingeCA[i]->setLimit(axil_direction_limit, axil_direction_limit);
      }
    }
  };
}
