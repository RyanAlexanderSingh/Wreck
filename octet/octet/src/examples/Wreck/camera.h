////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// mouse ball for rotating cameras around points of interest.

namespace octet {
  class camera : public resource {
    app *the_app;
    HWND w;
    const float sensitivity = -0.5f;
    vec3 cam_angle = (0.0f, 0.0f, 0.0f);

  public:
    camera() {
    }

    void init(app *app) {
      this->the_app = app;
      the_app->get_window_handle(w);
    }

    ///this function is responsible for moving the camera based on mouse position
    void update() {

      static bool is_mouse_moving = true;

      if (is_mouse_moving){

        int x = 0, y = 0;
        the_app->get_mouse_pos(x, y);
        int vx = 0, vy = 0;
        the_app->get_viewport_size(vx, vy);
        float dx = x - vx * 0.5f;
        float dy = y - vy * 0.5f;

        //apply the deltaX and deltaY of the mouse to the camera angles.
        cam_angle.x() += dx * sensitivity;
        cam_angle.y() += dy * sensitivity;
        is_mouse_moving = false;

        //set the position of the mouse to the center of the window
        tagPOINT p;
        p.x = vx * 0.5f;
        p.y = vy * 0.5f;
        ClientToScreen(w, &p);
        SetCursorPos(p.x, p.y);
      }
      else
      {
        is_mouse_moving = true;
      }
    }

    vec3 get_camera_angles(){
     return cam_angle;
    }
  };
}
