#include <Xinput.h>

namespace octet {

  class xbox_controller : public resource {

    //current controller state
    XINPUT_STATE controller_state;

    //index of the gamepad
    int controller_index;
    int A = 0; //action button A
    

  public:

    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
    float left_analog_x = 0.0f;
    float left_analog_y = 0.0f;
    float right_analog_x = 0.0f;
    float right_analog_y = 0.0f;
    

    xbox_controller()
    {
    }


    XINPUT_GAMEPAD *get_state(){
      return &controller_state.Gamepad;
    }

    bool is_connected()
    {
      int controllerId = -1;

      for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
      {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        if (XInputGetState(i, &state) == ERROR_SUCCESS)
          controllerId = i;
      }
      controller_index = controllerId;
      return controllerId != -1;
    }

    //adapted from the Arduino Map function
    float map_values(float x, float in_min, float in_max, float out_min, float out_max){
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    bool analog_deadzone(float analog_stick_x, float analog_stick_y){
      
      float check_analog_x = analog_stick_x;
      float check_analog_y = analog_stick_y;

      //left and right analog deadzone are the same, so we can just check against the left one, it makes no difference.
      //if the x axis is not in the deadzone, return false
      if (check_analog_x > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || check_analog_x < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE){
        return false;
      }

      //if the y axis is not in the deadzone, return false
      if (check_analog_y > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || check_analog_y < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE){
        return false;
      }
      
      //else it is in the deadzone, return true
      else{
        return true;
      }
    }

    bool button_a_pressed(){
      if (controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_A){
        return true;
      }
      else
      {
        return false;
      } 
    }

    bool button_b_pressed(){
      if (controller_state.Gamepad.wButtons & XINPUT_GAMEPAD_B){
        return true;
      }
      else
      {
        return false;
      }
    }


    bool refresh(){
        if (controller_index == -1){
          is_connected();
        }
        if (controller_index != -1)
        {
          ZeroMemory(&controller_state, sizeof(XINPUT_STATE));
          if (XInputGetState(controller_index, &controller_state) != ERROR_SUCCESS)
          {
            controller_index = -1;
            return false;
          }

          //check to see if the left analog stick is in the deadzone
          if (!analog_deadzone((float)controller_state.Gamepad.sThumbLX, (float)controller_state.Gamepad.sThumbLY)){
            left_analog_x = map_values((float)controller_state.Gamepad.sThumbLX, -32768, 32768, -0.261799f, 0.261799f);
          }

          //check to see if the right analog stick is in the deadzone
          if (!analog_deadzone((float)controller_state.Gamepad.sThumbRX, (float)controller_state.Gamepad.sThumbRY)){
            right_analog_x = map_values((float)controller_state.Gamepad.sThumbRX, -32768, 32768, -180, 180);
            right_analog_y = map_values((float)controller_state.Gamepad.sThumbRY, -32768, 32768, 180, -180);
          }
          
          left_trigger = map_values((float)controller_state.Gamepad.bLeftTrigger, 0.0f, 255.0f, 0.0f, 20.0f);
          right_trigger = map_values((float)controller_state.Gamepad.bRightTrigger, 0.0f, 255.0f, 0.0f, 20.0f);
          return true;
       }
       return false;
      }
        ~xbox_controller() {
        }
      };
    }