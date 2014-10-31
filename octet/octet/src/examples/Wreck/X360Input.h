#include <Xinput.h>

class X360Input{
private:
  //index of the gamepad
  int c_gamepadIndex;
  //current controller state
  XINPUT_STATE c_State;
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
    ZeroMemory(&c_State, sizeof(XINPUT_STATE));

    DWORD result = XInputGetState(c_gamepadIndex, &c_State);
    if (result == ERROR_SUCCESS)
      return true;
    else
      return false;
  }

  //return value of the left trigger for the controller
  float left_trigger(){
      BYTE trigger = c_State.Gamepad.bLeftTrigger;
      if (trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD){
        return trigger / 255.0f;
      }

  }

  //return the value of the right trigger for the controller
  float right_trigger(){}

  void init(int controllerIndex){
    c_gamepadIndex = controllerIndex - 1;
  }

  void update(){
    c_State = get_state(); //get the current gamepad state 
  }
};