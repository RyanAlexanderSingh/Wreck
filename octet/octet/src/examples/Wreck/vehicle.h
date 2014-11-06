////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {

  ///Class to create a vehicle using hinges
  class vehicle : public resource {

    xbox_controller xbox_controller;
    ALsource  test;

    app *the_app;
    visual_scene *app_scene;
    btDiscreteDynamicsWorld *the_world;

    dynarray<btRigidBody*> vehicles;
    dynarray<btRigidBody*> axils;

    //Axil-Wheel Hinges
    dynarray<btHingeConstraint*> hingeAW;

    //Chassis-Axil Hinges
    dynarray<btHingeConstraint*> hingeCA;

    float axil_direction_limit = 0.0f;
    float target_angular_velocity = 0.0f;
    float motor_velocity = 0.0f;

    //sounds
    ALuint start_engine;
    ALuint stop_engine;
    ALuint loop_engine;

    //sourced from Andy Thomason - Invaderers
    unsigned cur_source;
    ALuint sources[8];
    ALuint get_sound_source() { return sources[cur_source++ % 8]; }

    bool is_playing;

  public:

    dynarray<btRigidBody*> wheels;

    vehicle()
    {
    }

    void create_car_component(mat4t_in axilsize, mesh *msh, material *mtl, dynarray <btRigidBody*> *rbArray, bool is_dynamic, btScalar mass){
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

    void create_hinges(btRigidBody *rbA, btRigidBody *rbB, dynarray <btHingeConstraint*> *hinge_array, vec3_in PivotA, vec3_in PivotB, vec3_in Axis, bool set_hinge_limits){

      btVector3 btPivotA = get_btVector3(PivotA);
      btVector3 btPivotB = get_btVector3(PivotB);
      btVector3 btAxis = get_btVector3(Axis);

      btHingeConstraint *hingeConstraint = new btHingeConstraint((*rbA), (*rbB), btPivotA, btPivotB, btAxis, btAxis);
      if (set_hinge_limits){
        hingeConstraint->setLimit(0.0f, 0.0f);
      }
      hinge_array->push_back(hingeConstraint);
      the_world->addConstraint(hingeConstraint, false);
    }

    void sound_control(){
     

    }

    void init(app *app, visual_scene *app_scene, btDiscreteDynamicsWorld *world){
      this->the_app = app;
      this->app_scene = app_scene;
      this->the_world = world;

      mat4t modelToWorld;
      modelToWorld.translate(0.0f, 5.0f, 0.0f);
      vec3 chassis_size(3.0f, 0.125f, 2.0f);
      create_car_component(modelToWorld, new mesh_box(chassis_size), new material(vec4(1, 0, 1, 1)), &vehicles, true, 5.0f);

      vec3 axil_size(0.25f, 0.25f, 0.5f);
      material *wheel_mat = new material(new image("assets/tire.jpg"));
      material *red = new material(vec4(1, 0, 0, 1));

      for (float i = 0.0f; i != 4; ++i){
        modelToWorld.translate(i, 0.0f, 0.0f);
        create_car_component(modelToWorld, new mesh_cylinder(zcylinder(vec3(0, 0, 0), 1.0f, 0.5f)), wheel_mat, &wheels, true, 5.0f);
        create_car_component(modelToWorld, new mesh_box(axil_size), red, &axils, true, 20.0f);
      }

      float dist_x = chassis_size.x() - axil_size.x() * 2.0f;
      float dist_y = chassis_size.y();
      float dist_z = chassis_size.z() - axil_size.z();

      //create the hinges for the chassis - axils
      create_hinges(*&vehicles[0], *&axils[0], &hingeCA, vec3(dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[0], *&wheels[0], &hingeAW, vec3(0.0f, 0.0f, -0.575f), vec3(0.0f, 0.0f, 0.575f), vec3(0.0f, 0.0f, 1.0f), false);
      create_hinges(*&vehicles[0], *&axils[1], &hingeCA, vec3(dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, -dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[1], *&wheels[1], &hingeAW, vec3(0.0f, 0.0f, 0.575f), vec3(0.0f, 0.0f, -0.575f), vec3(0.0f, 0.0f, 1.0f), false);
      create_hinges(*&vehicles[0], *&axils[2], &hingeCA, vec3(-dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[2], *&wheels[2], &hingeAW, vec3(0.0f, 0.0f, -0.575f), vec3(0.0f, 0.0f, 0.575f), vec3(0.0f, 0.0f, 1.0f), false);
      create_hinges(*&vehicles[0], *&axils[3], &hingeCA, vec3(-dist_x, dist_y, 0.0f), vec3(0.0f, 0.0f, -dist_z), vec3(0.0f, 1.0f, 0.0f), true);
      create_hinges(*&axils[3], *&wheels[3], &hingeAW, vec3(0.0f, 0.0f, 0.575f), vec3(0.0f, 0.0f, -0.575f), vec3(0.0f, 0.0f, 1.0f), false);

      //sounds
      loop_engine = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/loop_engine.wav");
      cur_source = 0;
      alGenSources(8, sources);
    }

    ///any 
    void update(){

      const float step_acceleration = 0.2f;
      const float max_velocity = 20.0f;

      const float step_angle = 1.0f;
      const float max_angle = 15.0f;


      // rotation of the front two axils - turning the chassis left or right
      if (the_app->is_key_down('A') || the_app->is_key_down(key_left)) {
        if (axil_direction_limit > -(max_angle * 3.14159265f / 180.0f))
        {
          axil_direction_limit -= step_angle * 3.14159265f / 180.0f;
          rotate_axils(axil_direction_limit);
        }
      }
      if (the_app->is_key_down('D') || the_app->is_key_down(key_right)) {
        if (axil_direction_limit < (max_angle * 3.14159265f / 180.0f)){
          axil_direction_limit += step_angle * 3.14159265f / 180.0f;
          rotate_axils(axil_direction_limit);
        }
      }

      //moving the car forwards
      if (the_app->is_key_down('W') || the_app->is_key_down(key_up)){
        if (motor_velocity < max_velocity){
          motor_velocity += step_acceleration;
        }
      }

      //moving the car backwards - maximum velocity is less as we're moving backwards
      else if (the_app->is_key_down('S') || the_app->is_key_down(key_down)){
        if (motor_velocity > -max_velocity){
          motor_velocity -= step_acceleration;
        }
      }

      //if the xbox controller has been connected
      else if (xbox_controller.refresh()){
        //right trigger is acceleration, left trigger is decceleration.
        motor_velocity = xbox_controller.right_trigger - xbox_controller.left_trigger;
        //turn wheels based on x pos of left analog stick
        rotate_axils(xbox_controller.left_analog_x);
      }

      //if no force is being applied - lets create some fake friction and slow down the car
      if (motor_velocity != 0){
        if (motor_velocity > 0.0f){
          motor_velocity -= step_acceleration / 5;
        }
        if (motor_velocity < 0.0f){
          motor_velocity += step_acceleration / 5;
        }
      }

      move_direction(motor_velocity, 10);

      //close the progam
      if (the_app->is_key_down(key_esc))
      {
        exit(1);
      }
    }

    //used to control the car
    void move_direction(float motor_velocity, float motor_impulse_limit){
      for (int i = 0; i != 4; ++i){
        //optimize bullet simulation - don't want to waste memory on simulating static object
        axils[i]->activate(true);
        hingeAW[i]->enableAngularMotor(true, motor_velocity, motor_impulse_limit);
        sound_control();
      }

    }

    //rotate the front two axil's at an angle
    void rotate_axils(float axil_direction_limit){
      for (int i = 0; i != 2; ++i){
        axils[i]->activate(true);
        hingeCA[i]->setLimit(axil_direction_limit, axil_direction_limit);
      }
    }

    ~vehicle() {
    }
  };
}