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

		btDefaultCollisionConfiguration config;       /// setup for the world
		btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
		btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
		btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
		btDiscreteDynamicsWorld *world;             /// physics world, contains rigid bodies

		dynarray<btRigidBody*> rigid_bodies;
		dynarray<scene_node*> nodes;
		dynarray<btRigidBody*> wheels;
		dynarray<scene_node*> wheelnodes;

		dynarray<btRigidBody*> axils;
		dynarray<scene_node*> axilnodes;

		vec3 m_position;
		vec3 camAngle;
		float xMove;
		float zMove;

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
			btScalar mass = 10.0f;
			btVector3 inertiaTensor;

			shape->calculateLocalInertia(mass, inertiaTensor);
			btRigidBody *rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);

			rigid_body->setActivationState(DISABLE_DEACTIVATION);
			world->addRigidBody(rigid_body);
			rigid_bodies.push_back(rigid_body);

			mesh_box *box = new mesh_box(vec3(size));
			scene_node *node = new scene_node(modelToWorld, atom_);
			nodes.push_back(node);

			app_scene->add_child(node);
			material *floor_mat = new material(vec4(0, 1, 1, 1));
			app_scene->add_mesh_instance(new mesh_instance(node, box, floor_mat));
		}

		void add_wheels_and_axils(mat4t_in wheelresize, vec3_in wheel_size, mat4t_in axilresize,vec3_in axil_size){
			//WHEEL RIGID BODIES AND SCENE NODES

			btMatrix3x3 wheelMatrix(get_btMatrix3x3(wheelresize));
			btVector3 wheelPos(get_btVector3(wheelresize[3].xyz()));

			btCollisionShape *wheel_shape = new btCylinderShape(get_btVector3(wheel_size)); //collision shape for the wheel

			btTransform wheelTransform(wheelMatrix, wheelPos);

			btDefaultMotionState *wheelMotionState = new btDefaultMotionState(wheelTransform);
			btScalar mass = 10.0f;
			btVector3 inertiaTensor;

			wheel_shape->calculateLocalInertia(mass, inertiaTensor);
			btRigidBody *wheel = new btRigidBody(mass, wheelMotionState, wheel_shape, inertiaTensor);
			wheel->setActivationState(DISABLE_DEACTIVATION);
			world->addRigidBody(wheel);
			wheels.push_back(wheel);

			mesh_cylinder *m_cylinder = new mesh_cylinder(zcylinder(), wheelresize, 16);
			scene_node *wheelnode = new scene_node(wheelresize, atom_);
			wheelnodes.push_back(wheelnode);

			app_scene->add_child(wheelnode);
			material *wheel_mat = new material(vec4(0, 0, 0, 1));
			app_scene->add_mesh_instance(new mesh_instance(wheelnode, m_cylinder, wheel_mat));

			//AXIL RIGID BODIES AND SCENE NODES

			btMatrix3x3 axilMatrix(get_btMatrix3x3(axilresize));
			btVector3 axilPos(get_btVector3(axilresize[3].xyz()));

			btCollisionShape *axil_shape = new btBoxShape(get_btVector3(axil_size)); //collision shape for the axil

			btTransform axilTransform(axilMatrix, axilPos);

			btDefaultMotionState *axilMotionState = new btDefaultMotionState(axilTransform);

			axil_shape->calculateLocalInertia(mass, inertiaTensor);
			btRigidBody *axil = new btRigidBody(mass, axilMotionState, axil_shape, inertiaTensor);
			axil->setActivationState(DISABLE_DEACTIVATION);
			world->addRigidBody(axil);
			axils.push_back(axil);

			mesh_box *axilbox = new mesh_box(axil_size);
			scene_node *axilnode = new scene_node(axilresize, atom_);
			axilnodes.push_back(axilnode);

			app_scene->add_child(axilnode);
			material *axil_mat = new material(vec4(1, 0, 0, 1));
			app_scene->add_mesh_instance(new mesh_instance(axilnode, axilbox, axil_mat));
		}

		void makeCar(vec3_in chassis, vec3_in axil){

			//add the distance x,y,z for the car to wheels

			//Chassis to Axil - Hinge 1

			float dist_x = chassis.x() - axil.x();
			float dist_y = (3.0f * 0.5f) - (1.5f * 0.5f);
			float dist_z = (0.5f + 0.3f) * 0.5f;

			btVector3 PivotA(dist_x, dist_y, 0.0f);
			btVector3 PivotB(0.0f, 0.0f, -dist_z);

			btVector3 AxisA(0.0f, 0.0f, 1.0f);
			btVector3 AxisB(0.0f, 0.0f, 1.0f);

			btHingeConstraint *hingeAW_1 = new btHingeConstraint((*rigid_bodies[1]), (*axils[0]), PivotA, PivotB, AxisA, AxisB);
			world->addConstraint(hingeAW_1, true);


			//Axil to Wheel - Hinge 1
			//btVector3 AW_1(3, -1, 2.7);

			//btHingeConstraint *hingeAW_1 = new btHingeConstraint((*rigid_bodies[1]), (*wheels[0]), AW_1, btVector3(0, 0, 0), btVector3(2, 0, 3), btVector3(1, 0, 0), true);
			//world->addConstraint(hingeAW_1, true);

			////Axil to Wheel - Hinge 2
			//btVector3 AW_2(-3, -1, 2.7);

			//btHingeConstraint *hingeAW_2 = new btHingeConstraint((*rigid_bodies[1]), (*wheels[1]), AW_2, btVector3(0, 0, 0), btVector3(2, 0, 3), btVector3(1, 0, 0), true);
			//world->addConstraint(hingeAW_2, true);

			////Axil to Wheel - Hinge 3
			//btVector3 AW_3(3, -1, -2.7);

			//btHingeConstraint *hingeAW_3 = new btHingeConstraint((*rigid_bodies[1]), (*wheels[2]), AW_3, btVector3(0, 0, 0), btVector3(2, 0, 3), btVector3(1, 0, 0), true);
			//world->addConstraint(hingeAW_3, true);

			////Axil to Wheel - Hinge 4
			//btVector3 AW_4(-3, -1, -2.7);

			//btHingeConstraint *hingeAW_4 = new btHingeConstraint((*rigid_bodies[1]), (*wheels[3]), AW_4, btVector3(0, 0, 0), btVector3(2, 0, 3), btVector3(1, 0, 0), true);
			//world->addConstraint(hingeAW_4, true);
		}

		///this function is responsible for moving the camera based on mouse position
		void move_camera(int x, int y, HWND  *w)
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

				const float radius = 1.0f;
				const float radian = 3.14159265f / 180;

				xMove = radius *  cosf(camAngle.y() * radian) * sinf(camAngle.x() * radian);
				float yMove = radius * -sinf(dy * radian);
				zMove = radius * cosf(dx * radian) * cosf(dy * radian);

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
			world->setGravity(btVector3(0.0f, -50.0f, 0.0f));

			//load the scene and camera
			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			app_scene->get_camera_instance(0)->set_near_plane(1);
			app_scene->get_camera_instance(0)->set_far_plane(20000);
			app_scene->get_camera_instance(0)->get_node()->translate(vec3(0.0f, 10.0f, 0.0f));

			mat4t modelToWorld;
			material *floor_mat = new material(new image("assets/checkerboard.jpg"));

			// add the ground (as a static object)
			add_box(modelToWorld, vec3(200.0f, 0.5f, 200.0f), floor_mat, false);

			//add the car (a dynamic object)
			vec3 chassis(2.0f, 0.5f, 3.0f);
			add_car(modelToWorld, chassis);

			mat4t wheelresize;
			wheelresize.scale(-0.5, 1, 1);
			wheelresize.rotate(90, 0, 1, 0);
			wheelresize.translate(0.0f, 0.4f, 0.0f);
			vec3 axil(0.5f, 0.3f, 0.25f);
			for (int i = 0; i != 4; ++i){
				add_wheels_and_axils(wheelresize, vec3(0.5f, 1.0f, 1.0f), modelToWorld, axil);
			}

			makeCar(chassis, axil);

			// add the boxes (as dynamic objects)
			modelToWorld.translate(-4.5f, 10.0f, 0.0f);
			material *mat = new material(vec4(0, 1, 0, 1));
			//affects performance depending on size!
			math::random rand;
			for (int i = 0; i != 20; ++i) {
				modelToWorld.translate(3.0f, 0.0f, 0.0f);
				modelToWorld.rotateZ(360 / 20);
				float size = rand.get(0.1f, 1.0f);
				add_box(modelToWorld, vec3(size), mat);
			}
		}

		/// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {

			simulate();
			int vx = 0, vy = 0;
			get_viewport_size(vx, vy);
			app_scene->begin_render(vx, vy);

			world->stepSimulation(1.0f / 30);

			//update the rigid bodies
			for (unsigned i = 0; i != rigid_bodies.size(); ++i) {
				btRigidBody *rigid_body = rigid_bodies[i];
				btQuaternion btq = rigid_body->getOrientation();
				btVector3 pos = rigid_body->getCenterOfMassPosition();
				quat q(btq[0], btq[1], btq[2], btq[3]);
				//forming the modelToWorld matrix
				mat4t modelToWorld;
				if (i != 1)
				{
					modelToWorld = q;
				}
				else //camera is on rigidbody one - could change this
				{
					scene_node *cameraNode = app_scene->get_camera_instance(0)->get_node();
					nodes[i]->add_child(cameraNode);
					mat4t &cameraMatrix = cameraNode->access_nodeToParent();
					cameraNode->loadIdentity();
					cameraMatrix.translate(0, 10, -20);
					cameraMatrix.rotateY(camAngle.x());
					cameraMatrix.rotateX(camAngle.y() - 30);
				}
				modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);//position
				nodes[i]->access_nodeToParent() = modelToWorld;//apply to the node
			}

			//update the wheels
			for (unsigned i = 0; i != wheels.size(); ++i){
				btRigidBody *wheel = wheels[i];
				btQuaternion btq = wheel->getOrientation();
				btVector3 pos = wheel->getCenterOfMassPosition();
				quat q(btq[0], btq[1], btq[2], btq[3]);
				//forming the modelToWorld matrix
				mat4t modelToWorld;
				modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);//position
				wheelnodes[i]->access_nodeToParent() = modelToWorld;//apply to the node
			}

			//update the axils
			for (unsigned i = 0; i != 1; ++i){
				btRigidBody *axil = axils[i];
				btQuaternion btq = axil->getOrientation();
				btVector3 pos = axil->getCenterOfMassPosition();
				quat q(btq[0], btq[1], btq[2], btq[3]);
				//forming the modelToWorld matrix
				mat4t modelToWorld;
				modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);//position
				axilnodes[i]->access_nodeToParent() = modelToWorld;//apply to the node
			}

			// update matrices. assume 30 fps.
			app_scene->update(1.0f / 30);

			// draw the scene
			app_scene->render((float)vx / vy);
		}

		///
		void simulate()
		{
			moveCar();
			keyboardInputs();
		}

		void moveCar(){
			// movement keys
			if (is_key_down('A') || is_key_down(key_left)) {
				wheels[0]->applyCentralImpulse(btVector3(10, 0, 0));
				wheels[1]->applyCentralImpulse(btVector3(10, 0, 0));
			}
			if (is_key_down('D') || is_key_down(key_right)) {
				wheels[0]->applyCentralImpulse(btVector3(-10, 0, 0));
				wheels[1]->applyCentralImpulse(btVector3(-10, 0, 0));
			}
			if (is_key_down('W') || is_key_down(key_up))	{
				wheels[0]->applyCentralImpulse(btVector3(0, 0, 10));
				wheels[1]->applyCentralImpulse(btVector3(0, 0, 10));
			}
			if (is_key_down('S') || is_key_down(key_down)){
				wheels[0]->applyCentralImpulse(btVector3(0, 0, -10));
				wheels[1]->applyCentralImpulse(btVector3(0, 0, -10));
			}
			if (is_key_down(key_space)){
				wheels[0]->setFriction(10);
			}
		}

		///any random keyboard functions such as esc to close the game
		void keyboardInputs()
		{
			if (is_key_down(key_esc))
			{
				exit(1);
			}
		}
	};
}
