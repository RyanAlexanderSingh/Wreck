////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene using bullet for physics effects.
  class wreck_game : public app {

    enum car_direction{DIR_LEFT, DIR_RIGHT};
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
    dynarray<scene_node*> wheelnodes;

    dynarray<btRigidBody*> axils;
    dynarray<scene_node*> axilnodes;

    //Axil-Wheel Hinges
    dynarray<btHingeConstraint*> hingeAW;

    //Chassis-Axil Hinges
    dynarray<btHingeConstraint*> hingeCA;
    vec3 camAngle;

    float axil_direction_limit = 0.0f;
    float target_angular_velocity = 0.0f;
    float motor_velocity = 0.0f;

    const float max_angle = 7.0f;
    const float step_angle = 1.0f;

    void add_box(mat4t_in modelToWorld, vec3_in size, material *mat, bool is_dynamic = true) {
      btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
      btVector3 pos(get_btVector3(modelToWorld[3].xyz()));
      btCollisionShape *shape = new btBoxShape(get_btVector3(size));
      btTransform transform(matrix, pos);
      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btScalar mass = is_dynamic ? 1.0f : 0.0f;
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

    void add_car(mat4t_in modelToWorld, vec3_in size) {

      btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
      btVector3 pos(get_btVector3(modelToWorld[3].xyz()));

      btCollisionShape *shape = new btBoxShape(get_btVector3(size));

      btTransform transform(matrix, pos);

      btDefaultMotionState *motionState = new btDefaultMotionState(transform);

      btScalar mass = 5.0f;
      btVector3 inertiaTensor;
      shape->calculateLocalInertia(mass, inertiaTensor);

      btRigidBody *rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);
      rigid_body->setActivationState(DISABLE_DEACTIVATION);
      world->addRigidBody(rigid_body);
      rigid_bodies.push_back(rigid_body);

      mesh_box *box = new mesh_box(size);
      scene_node *node = new scene_node(modelToWorld, atom_);
      nodes.push_back(node);
      app_scene->add_child(node);
      material *floor_mat = new material(vec4(0, 1, 1, 1));

      app_scene->add_mesh_instance(new mesh_instance(node, box, floor_mat));

    }
    void addWheels(mat4t_in wheelsize, mesh *msh, material *mtl, bool is_dynamic){
      scene_node *node = new scene_node();
      node->access_nodeToParent() = wheelsize;
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, msh, mtl));
      wheelnodes.push_back(node);
      btMatrix3x3 matrix(get_btMatrix3x3(wheelsize));
      btVector3 pos(get_btVector3(wheelsize[3].xyz()));
      btCollisionShape *shape = msh->get_bullet_shape();

      if (shape){
        btTransform transform(matrix, pos);
        btDefaultMotionState *motionState = new btDefaultMotionState(transform);
        btScalar mass = 5.0f;
        btVector3 inertiaTensor;
        shape->calculateLocalInertia(mass, inertiaTensor);
        btRigidBody *wheel = new btRigidBody(mass, motionState, shape, inertiaTensor);
        wheel->setFriction(0.2);
        world->addRigidBody(wheel);
        wheel->setUserPointer(node);
        wheels.push_back(wheel);
      }
    }
    void addAxils(mat4t_in axilsize, mesh *msh, material *mtl, bool is_dynamic){
      scene_node *node = new scene_node();
      node->access_nodeToParent() = axilsize;
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, msh, mtl));
      axilnodes.push_back(node);
      btMatrix3x3 matrix(get_btMatrix3x3(axilsize));
      btVector3 pos(get_btVector3(axilsize[3].xyz()));
      btCollisionShape *shape = msh->get_bullet_shape();
      if (shape){
        btTransform transform(matrix, pos);
        btDefaultMotionState *motionState = new btDefaultMotionState(transform);
        btScalar mass = 5.0f;
        btVector3 inertiaTensor;
        shape->calculateLocalInertia(mass, inertiaTensor);
        btRigidBody *axil = new btRigidBody(mass, motionState, shape, inertiaTensor);
        world->addRigidBody(axil);
        axil->setUserPointer(node);
        axils.push_back(axil);
      }
    }

    void makeCar(){

      //add the distance x,y,z for the car to wheels

      const float hinge_limit_rear = 0.0f;
      float dist_x = 3.0f - 0.25f;
      float dist_y = 0.2f;
      float dist_z = 2.0f - 0.5f;

      btVector3 PivotA(dist_x, dist_y, 0.0f);
      btVector3 PivotB(0.0f, 0.0f, -dist_z);
      btVector3 CAAxis(0.0f, 1.0f, 0.0f);

      //Chassis to Axil - Hinge 1
      PivotA = btVector3(dist_x, dist_y, 0.0f);
      PivotB = btVector3(0.0f, 0.0f, dist_z);
      btHingeConstraint *hingeCA_1 = new btHingeConstraint((*rigid_bodies[1]), (*axils[0]), PivotA, PivotB, CAAxis, CAAxis);
      hingeCA_1->setLimit(axil_direction_limit, axil_direction_limit);
      hingeCA.push_back(hingeCA_1);
      world->addConstraint(hingeCA_1, true);

      //Axil to Wheel - Hinge 1
      btVector3 AWAxis(0.0f, 0.0f, 1.0f);
      btVector3 AW_1(0, 0, -0.575f);
      btHingeConstraint *hingeAW_1 = new btHingeConstraint((*axils[0]), (*wheels[0]), AW_1, btVector3(0.0f, 0.0f, 0.575f), AWAxis, AWAxis);
      hingeAW.push_back(hingeAW_1);
      world->addConstraint(hingeAW_1, true);

      //Chassis to Axil - Hinge 2
      PivotA = btVector3(dist_x, dist_y, 0.0f);
      PivotB = btVector3(0.0f, 0.0f, -dist_z);
      btHingeConstraint *hingeCA_2 = new btHingeConstraint((*rigid_bodies[1]), (*axils[1]), PivotA, PivotB, CAAxis, CAAxis);
      hingeCA_2->setLimit(axil_direction_limit, axil_direction_limit);
      hingeCA.push_back(hingeCA_2);
      world->addConstraint(hingeCA_2, true);

      //Axil to Wheel - Hinge 2
      btVector3 AW_2(0.0, 0, 0.575f);
      btHingeConstraint *hingeAW_2 = new btHingeConstraint((*axils[1]), (*wheels[1]), AW_2, btVector3(0, 0, -0.575f), AWAxis, AWAxis);
      hingeAW.push_back(hingeAW_2);
      world->addConstraint(hingeAW_2, true);

      //Chassis to Axil - Hinge 3
      PivotA = btVector3(-dist_x, dist_y, 0.0f);
      PivotB = btVector3(0.0f, 0.0f, dist_z);
      btHingeConstraint *hingeCA_3 = new btHingeConstraint((*rigid_bodies[1]), (*axils[2]), PivotA, PivotB, CAAxis, CAAxis);
      hingeCA_3->setLimit(axil_direction_limit, axil_direction_limit);
      hingeCA.push_back(hingeCA_3);
      world->addConstraint(hingeCA_3, true);

      //Axil to Wheel - Hinge 3
      btVector3 AW_3(0, 0, -0.575f);
      btHingeConstraint *hingeAW_3 = new btHingeConstraint((*axils[2]), (*wheels[2]), AW_3, btVector3(0.0f, 0.0f, 0.575f), AWAxis, AWAxis);
      hingeAW.push_back(hingeAW_3);
      world->addConstraint(hingeAW_3, true);

      //Chassis to Axil - Hinge 4
      PivotA = btVector3(-dist_x, dist_y, 0.0f);
      PivotB = btVector3(0.0f, 0.0f, -dist_z);
      btHingeConstraint *hingeCA_4 = new btHingeConstraint((*rigid_bodies[1]), (*axils[3]), PivotA, PivotB, CAAxis, CAAxis);
      hingeCA_4->setLimit(axil_direction_limit, axil_direction_limit);
      hingeCA.push_back(hingeCA_4);
      world->addConstraint(hingeCA_4, true);

      //Axil to Wheel - Hinge 4
      btVector3 AW_4(0, 0, 0.575f);
      btHingeConstraint *hingeAW_4 = new btHingeConstraint((*axils[3]), (*wheels[3]), AW_2, btVector3(0.0f, 0.0f, -0.575f), AWAxis, AWAxis);
      hingeAW.push_back(hingeAW_4);
      world->addConstraint(hingeAW_4, true);

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
      app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(0, 3, 20);

      mat4t modelToWorld;
      material *floor_mat = new material(new image("assets/floor.jpg"));
      // add the ground (as a static object)
      add_box(modelToWorld, vec3(200.0f, 0.5f, 200.0f), floor_mat, false);

      //add the car (a dynamic object)
      modelToWorld.loadIdentity();
      modelToWorld.rotate(90, 0, 1, 0);
      modelToWorld.translate(0, 5, 0);
      add_car(modelToWorld, vec3(3.0f, 0.1f, 2.0f));

      material *wheel_mat = new material(new image("assets/tire.jpg"));
      material *red = new material(vec4(1, 0, 0, 1));

      for (int i = 0; i != 4; ++i){
        addWheels(modelToWorld, new mesh_cylinder(zcylinder(vec3(0, 0, 0), 1.0f, 0.5)), wheel_mat, true);
        //addWheels(modelToWorld, new mesh_box(vec3(1.0f, 1.0f, 0.5f)), wheel_mat, true);
        addAxils(modelToWorld, new mesh_box(vec3(0.25f, 0.3f, 0.5f)), red, true);
      }

      makeCar();

      // add the boxes (as dynamic objects)
      modelToWorld.translate(-4.5f, 10.0f, 0);
      material *mat = new material(vec4(0, 1, 0, 1));
      //affects performance depending on size!
      math::random rand;
      /*for (int i = 0; i != 20; ++i) {
      modelToWorld.translate(3, 0, 0);
      modelToWorld.rotateZ(360 / 20);
      float size = rand.get(0.1f, 1.0f);
      add_box(modelToWorld, vec3(size), mat);
      }*/
      // comedy box
      /*modelToWorld.loadIdentity();
      modelToWorld.translate(0, 200, 0);
      add_box(modelToWorld, vec3(5.0f), floor_mat);*/
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
        hingeAW[i]->enableAngularMotor(true, motor_velocity, max_motor_impulse);
      }
    }

    void rotate_axils(float axil_direction_limit){
      for (int i = 0; i != 2; ++i){
        hingeCA[i]->setLimit(axil_direction_limit, axil_direction_limit);
      }
    }
  };
}
