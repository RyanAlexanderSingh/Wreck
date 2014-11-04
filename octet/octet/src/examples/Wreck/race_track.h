////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {

  ///Class to create a race_track using hinges
  class race_track : public resource {

    create_shape shape_creator;

  private:

    //dynarray<btRigidBody*> race_tracks;

  public:
    race_track()
    {
    }

    void init(){

      mat4t modelToWorld;
      //create our texture here for the road
      material *track_mat = new material(vec4(0, 0, 0, 1));

      shape_creator.shape_generator(modelToWorld, new mesh_box(vec3(200.0f, 0.5f, 200.0f)), track_mat);
    }

    ~race_track() {
    }
  };
}
