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
		btRigidBody *carBody;
		scene_node *cameraNode;
		scene_node *vehicle;
		
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
		void add_car(mat4t_in modelToWorld, vec3_in size, material *mat, bool is_dynamic = true) {

			btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
			btVector3 pos(get_btVector3(modelToWorld[3].xyz()));

			btCollisionShape *shape = new btBoxShape(get_btVector3(size));

			btTransform transform(matrix, pos);

			btDefaultMotionState *motionState = new btDefaultMotionState(transform);
			btScalar mass = is_dynamic ? 1.0f : 0.0f;
			btVector3 inertiaTensor;

			shape->calculateLocalInertia(mass, inertiaTensor);

			carBody = new btRigidBody(mass, motionState, shape, inertiaTensor);
			world->addRigidBody(carBody);

			mesh_box *box = new mesh_box(size);
			vehicle = new scene_node(modelToWorld, atom_);
			nodes.push_back(vehicle);

			app_scene->add_child(vehicle);
			app_scene->add_mesh_instance(new mesh_instance(vehicle, box, mat));
		}

		void move_camera(int x, int y, HWND  *w)
		{
			ShowCursor(false);
			static bool is_mouse_moving = true;

			if (is_mouse_moving){

				int vx, vy;
				get_viewport_size(vx, vy);
				float dx = x - vx * 0.5f;
				float dy = y - vy * 0.5f;

				const float sensitivity = -0.5f;
				camAngle.x() += dx * sensitivity;
				camAngle.y() += dy * sensitivity;

				const float radius = 10.0f;

				xMove = radius *  cosf(camAngle.y() * (3.14159265f / 180)) * sinf(camAngle.x() * (3.14159265f / 180));
				float yMove = radius * sinf(dy * (3.14159265f / 180));
				zMove = radius * cosf(dy * (3.14159265f / 180)) * cosf(dx * (3.14159265f / 180));
				printf("%f", yMove);
				//m_position.y() = yMove;

				is_mouse_moving = false;

				tagPOINT p;
				p.x = vx / 2;
				p.y = vy / 2;
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

			//load the scene and camera
			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			app_scene->get_camera_instance(0)->set_far_plane(20000);
			app_scene->get_camera_instance(0)->set_near_plane(1);
			cameraNode = app_scene->get_camera_instance(0)->get_node();
			cameraNode->translate(vec3(0, 0, 5));

			mat4t modelToWorld;
			material *floor_mat = new material(vec4(1, 1, 0.20f, 1));

			// add the ground (as a static object)
			add_box(modelToWorld, vec3(200.0f, 0.5f, 200.0f), floor_mat, false);

			//add our car, currently a box 
			material *carMat = new material(vec4(1, 2, 3, 4));
			add_car(modelToWorld, vec3(2.0f, 0.5f, 3.0f), carMat, true);

			// add the boxes (as dynamic objects)
			modelToWorld.translate(-4.5f, 10.0f, 0);
			material *mat = new material(vec4(0, 1, 0, 1));
			//affects performance depending on size!
			math::random rand;
			for (int i = 0; i != 20; ++i) {
				modelToWorld.translate(3, 0, 0);
				modelToWorld.rotateZ(360 / 20);
				float size = rand.get(0.1f, 1.0f);
				add_box(modelToWorld, vec3(size), mat);
			}

			// comedy box
			modelToWorld.loadIdentity();
			modelToWorld.translate(0, 200, 0);
			add_box(modelToWorld, vec3(5.0f), floor_mat);
		}

		/// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {

			simulate();
			int vx = 0, vy = 0;
			get_viewport_size(vx, vy);
			app_scene->begin_render(vx, vy);

			mat4t &camera = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
			
			camera.loadIdentity();

			camera.translate(0, 10, 20);
			camera.rotateY(camAngle.x());
			camera.rotateX(camAngle.y());

			world->stepSimulation(1.0f / 30);
			for (unsigned i = 0; i != rigid_bodies.size(); ++i) {
				btRigidBody *rigid_body = rigid_bodies[i];
				btQuaternion btq = rigid_body->getOrientation();
				btVector3 pos = rigid_body->getCenterOfMassPosition();
				quat q(btq[0], btq[1], btq[2], btq[3]);
				mat4t modelToWorld = q;
				modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);
				nodes[i]->access_nodeToParent() = modelToWorld;
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
			if (is_key_down(key_a) || is_key_down(key_left)) {
				m_position.x() += xMove;
			}
			if (is_key_down(key_d) || is_key_down(key_right)) {
				m_position.x() -= xMove;
			}
			if (is_key_down(key_w) || is_key_down(key_up))
			{
				m_position.z() -= zMove;
			}
			if (is_key_down(key_s) || is_key_down(key_down))
			{
				m_position.z() += zMove;
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
