namespace octet {

  class camera {
		mat4t cameraInstance;
		float angle, angleX;
		int preMouseX, preMouseY;
		int mouseX, mouseY;
		enum { MAX_MOVE = 2, MAX_ROTATION = 5 };

	public:
		camera () { angle = 0.0f; angleX = 0.0f; cameraInstance.loadIdentity(); cameraInstance.translate(0.0f, 15.0f, 0.0f); }
		// if I don't return reerence then I can't see anything because it is not modified that way
		mat4t& getCamera () { return cameraInstance; }
		float& getCameraAngle () { return angle; }
		void setCameraAngle ( float _angle ) { angle = _angle; }
		void translateLeft () { cameraInstance.translate(-1.0f/MAX_MOVE, 0.0f, 0.0f); }
		void translateRight () { cameraInstance.translate( 1.0f/MAX_MOVE, 0.0f, 0.0f); }
		void translateForward () { cameraInstance.translate( 0.0f, 0.0f, -1.0f/MAX_MOVE); }
		void translateBack () { cameraInstance.translate( 0.0f, 0.0f, 1.0f/MAX_MOVE); }
		void rotateLeft () { 
			cameraInstance.rotateX( -angleX );
			cameraInstance.rotateY( MAX_ROTATION );
			cameraInstance.rotateX( angleX );
		}
		void rotateRight () { 
			cameraInstance.rotateX( -angleX );
			cameraInstance.rotateY( -MAX_ROTATION );
			cameraInstance.rotateX( angleX );
		}
		void rotateUp () { 
			angleX += MAX_ROTATION;
			cameraInstance.rotateX( MAX_ROTATION ); 
		}
		void rotateDown () { 
			angleX -= MAX_ROTATION;
			cameraInstance.rotateX( -MAX_ROTATION );
		}
    void rotateDown ( float angle) { 
			angleX -= angle;
			cameraInstance.rotateX( -angle );
		}
		int& getMouseX () { return  mouseX; }
		int& getMouseY () { return  mouseY; }
		void savePreviousMousePos () { preMouseX = mouseX; preMouseY = mouseY; }
		void detectMouse () {
			if ( preMouseX < mouseX )
				rotateRight();
			else if ( preMouseX > mouseX )
				rotateLeft();
			if ( preMouseY < mouseY )
				rotateDown();
			else if ( preMouseY > mouseY )
				rotateUp();
			savePreviousMousePos();
		}
		void reset () {
			cameraInstance.loadIdentity();
			cameraInstance.translate(0.0f, 0.0f, 5.0f);
			angle = 0.0f;
			angleX = 0.0f;
		}
		void reset(float x, float y, float z) {
		    reset();
			cameraInstance.translate(x, y, z);
		}
	};

} // namespace octet