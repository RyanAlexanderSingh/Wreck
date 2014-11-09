////////////////////////////////////////////////////////////////////////////////
//
// Ryan Singh 2014
//
// vehicle.h - class creates a vehicle with a chassis, 4 axils and 4 wheels
// 

namespace octet {

  ///Class to create a vehicle using hinge constraints.
  ///Class will create a vehicle out of rigid bodies. The vehicle consists of a chassis, 4 axils and 4 wheels.
  ///They are attached using hinge constraints. There are 2 hinge constraint relationships, Chassis-Axils and Axils-Wheels.
  class vehicle : public resource {

    xbox_controller xbox_controller;

    app *the_app;
    visual_scene *app_scene;
    btDiscreteDynamicsWorld *the_world;

    dynarray<btRigidBody*> vehicles; //only one in array - should change
    dynarray<btRigidBody*> axils; //axil array - used to connect hinges and activate physics when movement occurs
    dynarray<btRigidBody*> wheels; //wheel array - used to connect hinges 

    dynarray<btHingeConstraint*> hingeAW; //Axil-Wheel Hinges
    dynarray<btHingeConstraint*> hingeCA; //Chassis-Axil Hinges

    float axil_direction_limit = 0.0f; //limit the axil can rotate in radians
    float motor_velocity = 0.0f; //speed in which the vehicle will move

    // helper for drawing text
    ref<text_overlay> overlay;

    // text mesh object for overlay.
    ref<mesh_text> text;

    //sourced from Andy Thomason
    //sounds
    ALuint loop_engine;
    unsigned cur_source;
    ALuint sources[8];
    ALuint get_sound_source() { return sources[cur_source++ % 8]; }

    bool key_down_last_frame = false;
    bool mute = false; //to mute the audio playing;

  public:

    vehicle()
    {
    }

    ///Function to create a car component, assigning a rigid body array and a mass.
    void create_car_component(mat4t_in axilsize, mesh *msh, material *mtl, dynarray <btRigidBody*> *rbArray, btScalar mass){
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

    ///Creates the hinge constraints between two rigid bodies. 
    /// Function takes in two rigid bodies, the hinge constraint it should be pushed back into, the pivots of the objects(position in local space to connect constraint),
    ///the axis in which the hinge constraint has freedom and a bool to check whether or not hinge limits should be applied. Hinge limits only applicable for Chassis-Axil hinge for rotation.
    void create_hinges(btRigidBody *rbA, btRigidBody *rbB, dynarray <btHingeConstraint*> *hinge_array, vec3_in PivotA, vec3_in PivotB, vec3_in Axis, bool set_hinge_limits){

      btVector3 btPivotA = get_btVector3(PivotA); //btVector3 of first rigid body pivot
      btVector3 btPivotB = get_btVector3(PivotB); //btVector3 of second rigid body pivot
      btVector3 btAxis = get_btVector3(Axis); //axis on which hinge constraint has freedome

      btHingeConstraint *hingeConstraint = new btHingeConstraint((*rbA), (*rbB), btPivotA, btPivotB, btAxis, btAxis);
      if (set_hinge_limits){
        hingeConstraint->setLimit(0.0f, 0.0f); //init the hinge constraint to 0 - can change when steering the vehicle
      }
      hinge_array->push_back(hingeConstraint);
      the_world->addConstraint(hingeConstraint, false); //do not disable collision between rigid bodies or they will ignore each other (very bad)
    }

    ///Function to play sound when the vehicle is moving. 
    /// sound_control gets the source state of selected source and returns it state
    /// if a sound file is not playing and the motor_velocity(movement) is != 0.0 then play a sound file.

    void sound_control(){

      unsigned source_state; //state of the selected source
      //get sound state of the selected sound source
      getSourceState(loop_engine, source_state);

      //init the sound
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, loop_engine);

      //if vehicle is moving in a direction, play the sound only if the sound is not already being played
      if (source_state != AL_PLAYING && motor_velocity != 0.0f && !mute){
        alSourcePlay(source);
      }
      //if the vehicle is not moving, stop the sound.
      else if (motor_velocity == 0.0f || mute){
        alSourceStop(source);
      }
    }

    /// Init the vehicle by creating the rigid bodies and attaching the hinge constraints.
    /// Creates the meshes and rigid bodies for the chassis, 4 axils and 4 wheels. Creates the hinge constraints for each of these rigid bodies.
    void init(app *app, visual_scene *app_scene, btDiscreteDynamicsWorld *world){
      this->the_app = app;
      this->app_scene = app_scene;
      this->the_world = world;

      // create the overlay
      overlay = new text_overlay();

      // get the defualt font.
      bitmap_font *font = overlay->get_default_font();

      // create a box containing text (in pixels)
      aabb bb(vec3(150.5f, -400.0f, 0.0f), vec3(256, 64, 0));
      text = new mesh_text(font, "", &bb);

      // add the mesh to the overlay.
      overlay->add_mesh_text(text);

      mat4t modelToWorld;
      modelToWorld.translate(0.0f, 5.0f, 0.0f);
      vec3 chassis_size(3.0f, 0.125f, 2.0f);
      create_car_component(modelToWorld, new mesh_box(chassis_size), new material(vec4(1, 0, 1, 1)), &vehicles, 5.0f);

      vec3 axil_size(0.25f, 0.25f, 0.5f);

      material *wheel_mat = new material(new image("assets/tire_test.jpg"));
      material *red = new material(vec4(1, 0, 0, 1));

      //create 4 wheels and 4 axils
      for (float i = 0.0f; i != 4; ++i){
        modelToWorld.translate(i, 0.0f, 0.0f);
        create_car_component(modelToWorld, new mesh_cylinder(zcylinder(vec3(0, 0, 0), 1.0f, 0.5f)), wheel_mat, &wheels, 5.0f);
        create_car_component(modelToWorld, new mesh_box(axil_size), red, &axils, 20.0f);
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

    ///Update the vehicle class every frame, called from the main app wreck_game. 
    /// Update takes care of the keyboard inputs and checks if an xbox controller has been connected.
    ///Keyboard inputs and the Xbox controller will not work together and keyboard inputs are taken as default. 
    /// If an Xbox controller is found then the inputs from the xbox controller are taken instead. 
    void update(int vx, int vy){

      const float step_acceleration = 0.4f; //float to increment motor_velocity
      const float max_velocity = 20.0f; //maximum velocity the vehicle can go

      const float step_angle = 1.5f; //float to increment the angle axils turn
      const float max_angle = 15.0f; //maximum angle the axil can rotate

      // write some text to the overlay
      char buf[3][256];

      text->clear();

      text->format("Sound muted: %s", mute ? "true" : "false");

      // convert it to a mesh.
      text->update();

      // draw the text overlay
      overlay->render(vx, vy);

      // rotation of the front two axils - turning the chassis left or right
      if (the_app->is_key_down('A') || the_app->is_key_down(key_left)) {
        if (axil_direction_limit > -(max_angle * 3.14159265f / 180.0f))
        {
          axil_direction_limit -= step_angle * 3.14159265f / 180.0f;
          rotate_axils(axil_direction_limit);
        }
      }

      else if (the_app->is_key_down('D') || the_app->is_key_down(key_right)) {
        if (axil_direction_limit < (max_angle * 3.14159265f / 180.0f)){
          axil_direction_limit += step_angle * 3.14159265f / 180.0f;
          rotate_axils(axil_direction_limit);
        }
      }

      else if (!the_app->is_key_down('A') || !the_app->is_key_down('D') || the_app->is_key_down(key_left) || the_app->is_key_down(key_right)){
        if (axil_direction_limit != 0){
          axil_direction_limit = 0.0f;
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

      //if no force is being applied - setting motor_velocity to 0 will produce a fake form of friction
      else if (!the_app->is_key_down('W') || !the_app->is_key_down('S') || the_app->is_key_down(key_up) || the_app->is_key_down(key_down)){
        if (motor_velocity != 0){
          motor_velocity = 0.0f;
        }
      }

      move_direction(motor_velocity, 10); //apply the motor_velocity

      //if the xbox controller has been connected
      if (xbox_controller.refresh()){
        //right trigger is acceleration, left trigger is decceleration.
        motor_velocity = xbox_controller.right_trigger - xbox_controller.left_trigger;
        move_direction(motor_velocity, 10);
        //turn wheels based on x pos of left analog stick
        if (!xbox_controller.analog_deadzone()){
          rotate_axils(xbox_controller.left_analog_x);
        }
        else{
          rotate_axils(0); //set the axils back to it's center point
        }
      }

      if (the_app->is_key_down('M') && !key_down_last_frame)
      {
        mute = !mute;
      }

        sound_control();

      //close the program
      if (the_app->is_key_down(key_esc))
      {
        exit(1); //exits the program....safely?
      }

      key_down_last_frame = the_app->is_key_down('M');
    }

    ///Move the vehicle by setting a velocity on the hinge angular motors. 
    /// To move the vehicle, all 4 axils are activated and the angular motor on all Axil-Wheel hinge constraints are enabled. 
    /// a motor velocity and a maximum impulse is then set to the hinge. 
    void move_direction(float motor_velocity, float motor_impulse_limit){
      for (int i = 0; i != 4; ++i){
        //optimize bullet simulation - don't want to waste memory on simulating static object
        axils[i]->activate(true);
        hingeAW[i]->enableAngularMotor(true, motor_velocity, motor_impulse_limit);
      }
    }

    ///Function to take in the radian at which to turn the vehicle. 
    /// The vehicle is turned by activating the front two axils in the scene and applying an angular rotation
    /// on the free angle in the Chassis-Axil hinge constraints 
    void rotate_axils(float axil_direction_limit){
      for (int i = 0; i != 2; ++i){
        //optimize bullet simulation - don't want to waste memory on simulating static object
        axils[i]->activate(true);
        hingeCA[i]->setLimit(axil_direction_limit, axil_direction_limit);
      }
    }

    ~vehicle() {
    }
  };
}