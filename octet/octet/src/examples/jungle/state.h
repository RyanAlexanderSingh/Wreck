namespace octet {

  class State : public octet::resource {
  public:
    virtual ~State() { };

    // Routine called upon entering the state (Optional implementation)
    virtual void onEnter() { };
    // Routine called upon exiting the state (Optional implementation)
    virtual void onExit() { };

    // Routine called on each tick
    virtual void operator()() = 0;
  };

  class StateMachine {
  public:
    typedef int StateID;

  private:
    octet::dynarray<octet::ref<State> > _states;
    octet::ref<State> _currentState;

  public:
    StateMachine() :
      _currentState(0) {
    };

    ~StateMachine() {
    };

    StateID registerState(State* state) {
      _states.push_back(state);
      return _states.size() - 1;
    };

    void setState(StateID state) {
      if (_currentState) {
        _currentState->onExit();
      }

      _currentState = (state > -1 && ((size_t) state) < _states.size() ? _states[state] : 0);

      if (_currentState) {
        _currentState->onEnter();
      }
    };

    void update() const {
      if (_currentState) {
        _currentState->operator()();
      }
    };
  };

}