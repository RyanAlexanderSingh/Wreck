namespace octet {

  /**
   *  Singleton template.
   *  Use with CRTP (http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
   **/
  template <typename T>
  class Singleton {
  protected:
    Singleton() {
    };

  public:
    virtual ~Singleton() {
    };

    static T& getInstance() {
      static T instance;
      return instance;
    };
  };
  
  template <typename T>
  T lerp(const T& begin, const T& end, float t) {
    return ((1.f - t) * begin) + (t * end);
  };

  // Reference: http://en.wikipedia.org/wiki/Smoothstep
  // Reference: http://sol.gfxile.net/interpolation/
  template <typename T>
  T smoothstep(const T& begin, const T& end, float t) {
    return lerp(begin, end, (t * t * (3.0f - 2.0f * t)));
  };
  
  // Reference: http://en.wikipedia.org/wiki/Smoothstep
  template <typename T>
  T smootherstep(const T& begin, const T& end, float t) {
    return lerp(begin, end, (t * t * t * (t * ((t * 6) - 15) + 10)));
  };

  // ^ p01 -- p11
  // |  |      |
  // |  |      |
  // V p00 -- p10
  //    U ---->
  template <typename T>
  T bilerp(const T& p00, const T& p10, const T& p11, const T& p01, float u, float v) {
    return lerp(lerp(p00, p10, u), lerp(p01, p11, u), v);
  };
  
  template <typename T>
  T bilerp(const T& p00, const T& p10, const T& p11, const T& p01, const octet::vec2& uv) {
    return bilerp(p00, p10, p11, p01, uv.x(), uv.y());
  };
  
  float square(float a) {
    return a * a;
  };

  float pi() {
    return 3.14159265f;
  };

  // Reference: http://en.wikipedia.org/wiki/Insertion_sort
  template <typename T, typename Comparator>
  void sort(octet::dynarray<T>& list, Comparator& comparator) {
    size_t j = 0;

    for (size_t i = 0; i < list.size(); ++i) {
      j = i;

      while (j > 0 && !comparator(list[j - 1], list[j])) {
        octet::swap(list[j - 1], list[j]);
        --j;
      }
    }
  };

  template <typename T>
  int contains(octet::dynarray<T>& list, const T& value) {
    for (size_t i = 0; i < list.size(); ++i) {
      if (list[i] == value) {
        return i;
      }
    }

    return -1;
  };
  
  /**
    *  Utility class which manages input.
    *  Used to delay key-press.
    */
  class InputManager {
  private:
    app* _app;

    // The key delay amount per poll request
    unsigned int _keyDelay;

    // Delay accumulator for processing key presses
    unsigned int _delay;

    // Non-copyable entity
    InputManager(const InputManager&);
    InputManager& operator=(const InputManager&);

  public:
    typedef unsigned int keycode;

    explicit InputManager(app* app = NULL, unsigned int keyDelay = 5) :
      _app(app),
      _keyDelay(keyDelay),
      _delay(0) {
    };

    ~InputManager() {
    };

    /**
      *  Attaches this manager instance with the
      *  specified app instance.
      *  @param app The app which requires input handling
      */
    void attach(app* app) {
      _app = app;
      _delay = 0;
    };
      
    /**
      *  Specify the key delay
      *  @param keyDelay The key delay
      */
    void setKeyDelay(unsigned int keyDelay) {
      _keyDelay = keyDelay;
      _delay = 0;
    };
      
    /**
      *  @param key The keycode to check for
      *  @return true if the specified key is pressed; false otherwise
      */
    bool isKeyDown(keycode key) const {
      return _delay == 0 && _app != NULL && _app->is_key_down(key);
    };
      
    /**
      *  @param key The keycode to check for
      *  @return true if the specified key is pressed and handled internally; false otherwise
      */
    bool processKey(keycode key) {
      if (isKeyDown(key)) {
        _delay = _keyDelay;
        return true;
      };

      return false;
    };
      
    /**
      *  Check for new key presses
      */
    void poll() {
      _delay = (_delay == 0 ? 0 : _delay - 1);
    };
  };

} // namespace octet