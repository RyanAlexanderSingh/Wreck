namespace octet {

  class CameraHelper {
		mat4t* cameraToWorld;

		float angleX, angleY;
    float _translationFactor;
    float _rotationFactor;
    dynarray<vec2> positions;

    static float getDefaultTranslationFactor() {
      return 10.0f;
    };
    
    static float getDefaultRotationFactor() {
      return 5.0f;
    };

/*
    mat4t& getDummyCamera() const {
      static mat4t camera;

      // Always reset dummy camera
      camera.loadIdentity();
      return camera;
    };
*/

	public:
		CameraHelper() :
      cameraToWorld(NULL),
      angleX(0.0f), angleY(0.0f),
      _translationFactor(getDefaultTranslationFactor()),
      _rotationFactor(getDefaultRotationFactor()) {
    };

		CameraHelper(camera_instance* cameraInstance) :
      cameraToWorld(&(cameraInstance->get_node()->access_nodeToParent())),
      angleX(0.0f), angleY(0.0f),
      _translationFactor(getDefaultTranslationFactor()),
      _rotationFactor(getDefaultRotationFactor()) {
    };

		CameraHelper(mat4t* cameraToWorld) :
      cameraToWorld(cameraToWorld),
      angleX(0.0f), angleY(0.0f),
      _translationFactor(getDefaultTranslationFactor()),
      _rotationFactor(getDefaultRotationFactor()) {
    };
      
    void setCamera(camera_instance* cameraInstance) {
      setCamera(&(cameraInstance->get_node()->access_nodeToParent()));
    };

    void setCamera(mat4t* cameraToWorld) {
      this->cameraToWorld = cameraToWorld;
    };

    float getTranslationFactor() const {
      return _translationFactor;
    };

    void setTranslationFactor(float translationFactor) {
      _translationFactor = translationFactor;
    };

    float getRotationFactor() const {
      return _rotationFactor;
    };
    
    void setRotationFactor(float rotationFactor) {
      _rotationFactor = rotationFactor;
    };

    void pushPosition ( vec2 &v ) { positions.push_back(v); }
    void resetPositions ( ) { positions.reset(); }

		// if I don't return reerence then I can't see anything because it is not modified that way
		mat4t& getCamera () {
      // Safe version
      //return (cameraToWorld == NULL ? getDummyCamera() : *cameraToWorld);

      return *cameraToWorld;
    };

    const mat4t& getCamera () const {
      return *cameraToWorld;
    };

    void moveTo () {
      static int currentTargetPosition = 0;
      //printf( " Centers %d pos %f %f\n", currentTargetPosition, positions[currentTargetPosition].x(),  positions[currentTargetPosition].y() );
      // atan2 returns radians
      float angle = atan2( positions[currentTargetPosition].y()-getCamera().w().z(), positions[currentTargetPosition].x()-getCamera().w().x() )*180.0f/3.14f; // angle between two points
      getCamera().translate( cos(angle)*0.2f, 0.0f, sin(angle)*0.2f);
      rotateAtAngle (0.1f) ;
      //rotateAtAngle (0.1f) ;
      //rotateAtAngle(0.5f);
      //printf( "camera pos %f %f\n", getCamera().w().x(),  getCamera().w().z() );
      // if camera is almost at the destination then change the destination to the next one
      if ( fabs(getCamera().w().x()-positions[currentTargetPosition].x()) < 2.5f && fabs(getCamera().w().z()-positions[currentTargetPosition].y()) < 2.5f ) {
        // if its at the end of the dynamic array then make it 0 otherwise increae by 1
        currentTargetPosition = ( currentTargetPosition == (positions.size()-1)) ? 0: ++currentTargetPosition;
        
        //setRotation(angle*180.0f/3.14f);
      }
    }

		void translateLeft () { getCamera().translate(-getTranslationFactor(), 0.0f, 0.0f); }
		void translateRight () { getCamera().translate(getTranslationFactor(), 0.0f, 0.0f); }
		void translateForward () { getCamera().translate( 0.0f, 0.0f, -getTranslationFactor()); }
		void translateBack () { getCamera().translate( 0.0f, 0.0f, getTranslationFactor()); }
    void setRotation (float angle) {
      resetRotation();
      getCamera().rotateY( angle );
    }
    void rotateAtAngle ( float angle ) {
      // if the camera doesnt have the angle that is supposed to
      //if ( angle != angleY ) {
        getCamera().rotateX( -angleX );
        // place it at ) rotation around Y axis
       // getCamera().rotateY( -angleY );
        // rotate it where is should be
			  getCamera().rotateY( angle );
			  getCamera().rotateX( angleX );
      //  angleY = angle;
      //}
    }
		void rotateLeft () { 
      angleY += getRotationFactor();
			getCamera().rotateX( -angleX );
			getCamera().rotateY( getRotationFactor() );
			getCamera().rotateX( angleX );
		}
		void rotateRight () { 
      angleY -= getRotationFactor();
			getCamera().rotateX( -angleX );
			getCamera().rotateY( -getRotationFactor() );
			getCamera().rotateX( angleX );
		}
		void rotateUp () { 
			angleX += getRotationFactor();
			getCamera().rotateX( getRotationFactor() ); 
		}
		void rotateDown () { 
			angleX -= getRotationFactor();
			getCamera().rotateX( -getRotationFactor() );
		}
    void rotateDown ( float angle) { 
			angleX -= angle;
			getCamera().rotateX( -angle );
		}
    void resetRotation () {
      vec3 translation = getCamera().w();
      getCamera().loadIdentity();
			getCamera().translate(translation.x(), translation.y(),translation.z());
			angleX = 0.0f;
    }
		void reset () {
			reset(0.0f, 0.0f, 5.0f);
		}
		void reset(float x, float y, float z) {
			getCamera().loadIdentity();
			getCamera().translate(x, y, z);
			angleX = 0.0f;
		}
	};
  
  class CameraKeyboardHandler {
  private:
    app* _app;
    CameraHelper _camera;
    enum state { NORMAL, FREE} _state;
    int fogState;
    
  public:
    
    CameraKeyboardHandler() :
      _app(NULL)  { _state = NORMAL; fogState = 0; };

    const int getFogState () const {
      return fogState;
    }

    void attach(app* app) {
      _app = app;
    };
    
    const CameraHelper& getCameraHelper() const {
      return _camera;
    };

    CameraHelper& getCameraHelper() {
      return _camera;
    };
    
    const mat4t& getCamera() const {
      return getCameraHelper().getCamera();
    };
    
    mat4t& getCamera() {
      return getCameraHelper().getCamera();
    };

    void setCamera(camera_instance* cameraInstance) {
      getCameraHelper().setCamera(cameraInstance);
    };

    void setCamera(mat4t* cameraToWorld) {
      getCameraHelper().setCamera(cameraToWorld);
    };

    void pushVertexPosition ( vec2 &v ) { getCameraHelper().pushPosition(v); }
    void resetPositions () { _camera.resetPositions(); }

    void update() {

      if ( _state == FREE ) {
        getCameraHelper().moveTo();
      }

      // translate camera left
		  if (_app->is_key_down('A')) {
			  getCameraHelper().translateLeft();
		  }
		  // translate camera right
		  if (_app->is_key_down('D')) {
			  getCameraHelper().translateRight();
		  }
		  // translate camera forward
		  if (_app->is_key_down('W')) {
			  getCameraHelper().translateForward();
		  }
		  // translate camra backwards
		  if (_app->is_key_down('S')) {
			  getCameraHelper().translateBack();
		  }
		 
      // pan camera up
		  if (_app->is_key_down(key_up)) {
			  getCameraHelper().rotateUp();
		  }
		  // pan camera down
		  if (_app->is_key_down(key_down)) {
			  getCameraHelper().rotateDown();
		  }
		  // pan camera left
		  if (_app->is_key_down(key_left)) {
			  getCameraHelper().rotateLeft();
		  }
		  // pan camera right
		  if (_app->is_key_down(key_right)) {
			  getCameraHelper().rotateRight();
		  }

      // free flying camera
      if (_app->is_key_down('F')) {
        getCameraHelper().resetRotation();
        _state = FREE;
        Sleep(100);
		  }
      // normal camera movement with keys
      if (_app->is_key_down('N')) {
        _state = NORMAL;
		  }
      // fog
      if (_app->is_key_down('Z')) {
			  fogState = (fogState == 0 ) ? 1 : 0;
        Sleep(100);
		  }

      // reset camera
		  if (_app->is_key_down(key_space)) {
			  getCameraHelper().reset();
		  }
    };
  };

} // namespace octet