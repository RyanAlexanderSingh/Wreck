#include <Xinput.h>

class X360Input{
private:
  //index of the gamepad
  int c_gamepadIndex;
  //current controller state
  XINPUT_STATE controller_state;
public:

  //get the state of the controller
  XINPUT_STATE get_state(){
    //temp controller state to return
    XINPUT_STATE gamepadState;
    //zero memory
    ZeroMemory(&gamepadState, sizeof(XINPUT_STATE));
    //get state of current controller
    XInputGetState(c_gamepadIndex, &gamepadState);
    //return controller state
    return gamepadState;
  }

  //return controller index
  int get_index(){
    return c_gamepadIndex;
  }

  bool is_connected(){
    //Zero memory
    ZeroMemory(&controller_state, sizeof(XINPUT_STATE));

    DWORD result = XInputGetState(c_gamepadIndex, &controller_state);
    if (result == ERROR_SUCCESS)
      return true;
    else
      return false;
  }

  //return value of the left trigger for the controller
  float left_trigger(){
      BYTE trigger = controller_state.Gamepad.bLeftTrigger;
      if (trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD){ //above threshold - therefore it counts as an action
        return trigger / 255.0f;
      }
      return 0.0f; //the trigger was not pressed
  }

  //return the value of the right trigger for the controller
  float right_trigger(){
    BYTE trigger = controller_state.Gamepad.bRightTrigger;
    if (trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD){ //above threshold - therefore it counts as an action
      return trigger / 255.0f;
    }
    return 0.0f; //the trigger was not pressed
  }

  void init(int controllerIndex){
    c_gamepadIndex = controllerIndex - 1;
  }

  void update(){
    controller_state = get_state(); //get the current gamepad state 
  }
};