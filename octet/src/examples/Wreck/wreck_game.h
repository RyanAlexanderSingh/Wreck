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
    
    mouse_ball mouseBall;
		car Car;
		btDefaultCollisionConfiguration config;       /// setup for the world
		btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
		btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
		btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
		btDiscreteDynamicsWorld *world;             /// physics world, contains rigid bodies

		dynarray<btRigidBody*> rigid_bodies;
		dynarray<scene_node*> nodes;


		float distance;
		bool was_mouse_down;
		int prev_x;
		int prev_y;
		float sensitivity;
		float angleX;



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
			scene_node *node = new scene_node(modelToWorld, atom_);
			nodes.push_back(node);

			app_scene->add_child(node);
			app_scene->add_mesh_instance(new mesh_instance(node, box, mat));

		}
	public:
		/// this is called when we construct the class before everything is initialised.
		wreck_game(int argc, char **argv) : app(argc, argv) {
			dispatcher = new btCollisionDispatcher(&config);
			broadphase = new btDbvtBroadphase();
			solver = new btSequentialImpulseConstraintSolver();
			world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
			was_mouse_down = false;
			sensitivity = 200;
		}

		~wreck_game() {
			delete world;
			delete solver;
			delete broadphase;
			delete dispatcher;
		}

		/// this is called once OpenGL is initialized
		void app_init() {

      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(20000);
      app_scene->get_camera_instance(0)->set_near_plane(1);
      
      mouseBall.init(this, 10, 200);
			
			cameraNode = app_scene->get_camera_instance(0)->get_node();
			cameraNode->translate(vec3(0, 5, 0));

      camera = app_scene->get_camera_instance(0);

			mat4t modelToWorld;
			material *floor_mat = new material(vec4(0, 2, 3, 3));
			// add the ground (as a static object)
			add_box(modelToWorld, vec3(200.0f, 0.5f, 200.0f), floor_mat, false);

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

      distance = 5;
		}



		/// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			simulate();

			int vx = 0, vy = 0;
			get_viewport_size(vx, vy);
			app_scene->begin_render(vx, vy);

      mat4t cameraToWorld = cameraNode->calcModelToWorld();
      //update(cameraToWorld);
      mouseBall.update(cameraToWorld);
      app_scene->get_camera_instance(0)->set_cameraToWorld(cameraToWorld, (float)vx / vy);
   
      
      bool is_mouse_down = is_key_down(key_lmb) && is_key_down(key_alt);
      if (is_mouse_down) {
        int x = 0, y = 0;
        int vx = 0, vy = 0;
        get_mouse_pos(x, y);
        get_viewport_size(vx, vy);
        if (was_mouse_down && vx && vy) {
          // spin the camera around the target
          float cx = vx * 0.5f;
          float cy = vy * 0.5f;
          float pfx = (prev_x - cx) / vx;
          float pfy = (prev_y - cy) / vy;
          float fx = (x - cx) / vx;
          float fy = (y - cy) / vy;
          vec4 dxy(pfy - fy, pfx - fx, 0, 0);
          float len2 = dxy.squared();
          if (len2 > 0) {
            vec4 norm = dxy.normalize();
            vec3 cameraNorm = (norm.x(), norm.y(), 0);
            cameraToWorld.translate(0, 0, -distance);
            cameraToWorld.rotate(sqrtf(len2) * sensitivity, norm.x(), norm.y(), 0);
            cameraToWorld.translate(0, 0, distance);
          }
        }
        prev_x = x;
        prev_y = y;
      }
      was_mouse_down = is_mouse_down;

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
    
    ///move the camera around when the left mouse button is down
    void update(mat4t &cameraToWorld){
    distance = 5;
      bool is_mouse_down = is_key_down(key_lmb) && is_key_down(key_alt);
      if (is_mouse_down) {
        int x = 0, y = 0;
        int vx = 0, vy = 0;
        get_mouse_pos(x, y);
        get_viewport_size(vx, vy);
        if (was_mouse_down && vx && vy) {
          // spin the camera around the target
          float cx = vx * 0.5f;
          float cy = vy * 0.5f;
          float pfx = (prev_x - cx) / vx;
          float pfy = (prev_y - cy) / vy;
          float fx = (x - cx) / vx;
          float fy = (y - cy) / vy;
          vec4 dxy(pfy - fy, pfx - fx, 0, 0);
          float len2 = dxy.squared();
          if (len2 > 0) {
            vec4 norm = dxy.normalize();
            vec3 cameraNorm = (norm.x(), norm.y(), 0);
            cameraToWorld.translate(0, 0, -distance);
            cameraToWorld.rotate(sqrtf(len2) * sensitivity, norm.x(), norm.y(), 0);
            cameraToWorld.translate(0, 0, distance);
          }
        }
        prev_x = x;
        prev_y = y;
      }
      was_mouse_down = is_mouse_down;
    }
		///
		void simulate()
		{
      //moveCamera();
			moveCar();
      keyboardInputs();
		}

	
		///
		void moveCar(){
			//const float ship_speed = 0.05f;
			// movement keys
			if (is_key_down(key_a) || is_key_down(key_left)) {
				cameraNode->translate(vec3(-1, 0, 0));
			}
			if (is_key_down(key_d) || is_key_down(key_right)) {
				cameraNode->translate(vec3(1, 0, 0));
			}
			if (is_key_down(key_w) || is_key_down(key_up))
			{
				cameraNode->translate(vec3(0, 0, -1));
			}
			if (is_key_down(key_s) || is_key_down(key_down))
			{
				cameraNode->translate(vec3(0, 0, 1));
			}
		}

    void keyboardInputs()
    {
      if (is_key_down(key_esc))
      {
        exit(1);
      }
    }

	};
}
