////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
	/// Scene using bullet for physics effects.
	class car : public resource {
		// scene for drawing box
    app *theapp;

		ref<visual_scene> app_scene;

		dynarray<btRigidBody*> rigid_bodies;
		dynarray<scene_node*> nodes;

	public:
		/// this is called when we construct the class before everything is initialised.
    car ()
    {
      
		}

		~car() {
		}
    /*
		/// this is called once OpenGL is initialized
		void app_init() {
			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 5, 0));

			mat4t modelToWorld;
			material *floor_mat = new material(vec4(0, 1, 3, 1));

			// add the ground (as a static object)

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
      
		}

    void moveBox(){
      const float ship_speed = 0.05f;
      // left and right arrows
      if (is_key_down(key_left)) {
      }
      else if (is_key_down(key_right)) {
      }
    }
    */
	};
}
