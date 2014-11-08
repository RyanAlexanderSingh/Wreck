////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include <stdio.h>
#include <stdlib.h>

namespace octet {

  ///Class to create a race_track using hinges
  class race_track : public resource {

    app *the_app;
    visual_scene *app_scene;
    btDiscreteDynamicsWorld *the_world;

  private:

  public:
    race_track()
    {
    }

    void create_track_component(mat4t_in track_size, mesh *msh, material *mtl, bool rigid_body){

      scene_node *track_nodes = new scene_node();
      track_nodes->access_nodeToParent() = track_size;
      app_scene->add_child(track_nodes);
      app_scene->add_mesh_instance(new mesh_instance(track_nodes, msh, mtl));

      if (rigid_body){
        btMatrix3x3 matrix(get_btMatrix3x3(track_size));
        btVector3 pos(get_btVector3(track_size[3].xyz()));
        btCollisionShape *shape = msh->get_bullet_shape();

        btTransform transform(matrix, pos);
        btDefaultMotionState *motionState = new btDefaultMotionState(transform);
        btVector3 inertiaTensor;
        shape->calculateLocalInertia(0.0f, inertiaTensor);
        btRigidBody *track = new btRigidBody(0.0f, motionState, shape, inertiaTensor);
        track->setFriction(10);
        the_world->addRigidBody(track);
        track->setUserPointer(track_nodes);
      }
    }

    void init(app *app, visual_scene *app_scene, btDiscreteDynamicsWorld *world){
      this->the_app = app;
      this->app_scene = app_scene;
      this->the_world = world;

      //create the fake skybox
      mat4t skybox_m;
      //skybox_m.rotateX(-90);
      material *skybox_mat = new material(new image("assets/seamless_sky.jpg"));
      create_track_component(skybox_m, new mesh_box(vec3(500.0f, 200.0f, 500.0f)), skybox_mat, false);

      //create the roads
      mat4t modelToWorld;
      //create our texture here for the road
      create_track_component(modelToWorld, new mesh_box(vec3(400.0f, 0.5f, 400.0f)), new material(new image("assets/floor.jpg")), true);
      material *track_mat = new material(new image("assets/road_texture.jpg"));

      dynarray<unsigned char> file;
      app_utils::get_url(file, "assets/race_track.txt");
      
      dynarray<char> single_variable;
      dynarray<float> track_specifications;
      //there are 10 values per line, 4 for the rotation, 3 for the translation and 3 for the size of the mesh
      for (unsigned i = 0; i != file.size(); ++i){
        unsigned c = file[i];
        //33 in UTF-8 represents an exclaimation mark, our breakpoint for reading the whole file.
        if (c != 33){
          //commas are used to separate the values
          if (c != 44 && c!= 10 && c != 13 && c != 92){
            single_variable.push_back(c); //if no comma, it's part of the same variable
          }
          if (c == 44){
            single_variable.push_back('\0'); //null terminate the buffer
            //parse char string to double and assign it to a float 
            float value = strtod(single_variable.data(), NULL);
            track_specifications.push_back(value);
            single_variable.reset();
          }
          //put all the stored information in as a newline approaches
          if (c == 10){      
            track_specifications.push_back('\0'); //null terminate the buffer  
            modelToWorld.rotate(track_specifications[0], track_specifications[1], track_specifications[2], track_specifications[3]);
            modelToWorld.translate(track_specifications[4], track_specifications[5], track_specifications[6]);
            create_track_component(modelToWorld, new mesh_box(vec3(track_specifications[7], track_specifications[8], track_specifications[9])), track_mat, true);
            for (int i = 0; i != track_specifications.size(); ++i){
              printf("%f \n", track_specifications[i]);
            }
            
            track_specifications.reset();
          }
        }
      }

      //modelToWorld.translate(0.0f, 2.0f, 0.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(50.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotate(90, 0, 1, 0);
      //modelToWorld.translate(60.0f, 0.0f, 60.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(70.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.translate(0.0f, 0.0f, 20.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(70.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateX(-10);
      //modelToWorld.translate(65.0f, 0.0f, 0.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(20.0f, 0.5f, 10.0f)), track_mat, true);
      ////deadend on the left hand side to the ramp
      //mat4t deadend = modelToWorld;
      //deadend.rotateX(-80);
      //deadend.translate(40.0, 20.0f, 0.0f);
      //material *deadend_mat = new material(new image("assets/deadend.jpg"));
      //create_track_component(deadend, new mesh_box(vec3(5.0f, 50.0f, 10.0f)), deadend_mat, true);
      //modelToWorld.rotateX(10);
      //modelToWorld.translate(59.5f, 3.45f, 0.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(40.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateY(-90);
      //modelToWorld.translate(60.0f, 0.0f, -50.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(70.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateY(90);
      //modelToWorld.translate(40.0f, 0.0f, 60.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(30.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateY(90);
      //modelToWorld.translate(5.0f, 0.0f, 40.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(15.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateZ(10);
      //modelToWorld.translate(50.0f, -2.0f, 0.0);
      //create_track_component(modelToWorld, new mesh_box(vec3(50.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateZ(-10);
      //modelToWorld.translate(98.8f, 8.6f, 0.0);
      //create_track_component(modelToWorld, new mesh_box(vec3(50.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateZ(-15);
      //modelToWorld.translate(88.0f, 13.0f, 0.0);
      //create_track_component(modelToWorld, new mesh_box(vec3(40.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateZ(15);
      //modelToWorld.translate(60.0f, -10.0f, 0.0);
      //create_track_component(modelToWorld, new mesh_box(vec3(30.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateY(-90);
      //modelToWorld.translate(-70.0f, 0.0f, -40.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(80.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateY(90);
      //modelToWorld.translate(-30.0f, 0.0f, -70.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(20.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateY(-90);
      //modelToWorld.translate(-100.0f, 0.0f, 30.0f);
      //create_track_component(modelToWorld, new mesh_box(vec3(110.0f, 0.5f, 10.0f)), track_mat, true);
      //modelToWorld.rotateY(-90);
      //modelToWorld.rotateZ(-3.5);
      //modelToWorld.translate(12.0f, 0.75f, 74.55f);
      //create_track_component(modelToWorld, new mesh_box(vec3(20.0f, 0.5f, 10.0f)), track_mat, true);
    }

    void update(){

    }

    ~race_track() {
    }
  };
}