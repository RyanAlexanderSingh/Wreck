


namespace octet {
  class HandleMesh {
		mat4t modelToWorld;
		float halfWidth;
	    float halfHeight;

		vec4 color;

		Mesh mesh_;
		dynarray<mat4t> modelToWorldMatrices;

    	GLuint texture_handle_;         // this is an OpenGL texture handle which is used to draw the image.

		//color_shader color_shader_;
		//texture_shader texture_shader_;

    // Used to avoid texture bleeding
    static float getTextureOffset() {
      return 0.02f;
    };

	public:
		HandleMesh () { }

		void init() {
		  mesh_.init();
		 }
    
		const Mesh& getMesh() const {
		  return mesh_;
		};

		Mesh& getMesh() {
		  return mesh_;
		};
    
    float getHeight() { return halfHeight*2; }
		void rotateY ( float angle ) { modelToWorld.rotateY(angle); }
		void rotateX ( float angle ) { modelToWorld.rotateX(angle); }
		void translate(float x, float y, float z) { modelToWorld.translate(x, y, z); }
		void rotateZ(float angle) { modelToWorld.rotateZ(angle); }

    void position ( mat4t& transform ) {
      modelToWorld = transform;
    }

		// to load new tree we need the world to be the identity matrix and we translate it in the right position (x,y,z)
		// before we add the vertices into the mesh
		void loadNewObject (float x, float y, float z, float w, float h) {
		  modelToWorld.loadIdentity();
		  modelToWorld.translate(x, y, z);
		  halfWidth = w*0.5f;
		  halfHeight = h*0.5f;
		  color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}

		mat4t &getLastModelToWorldmatrix () { return modelToWorldMatrices.back(); }
		mat4t &getModelToWorldMatrix () { return modelToWorld; };

		void pushVertex (vec3 &v) {
			mesh_.pushVertex(v);
		}

    void pushIndex () { mesh_.pushIndex(); }


		void pushUV ( float u, float v ) { mesh_.pushUV( u , v ); }

		// push index for quad that form 2 triangles
		// The indexing is placed in a clockwise manner
		void pushIndexForQuad() {
			mesh_.pushIndex(); // 0 + lastIndex
			mesh_.pushIndex(); // 1 + lastIndex
			mesh_.pushIndex(); // 2 + lastIndex

			mesh_.pushIndex(mesh_.getIndexNumber()-2); // 1 + lastIndex
			mesh_.pushIndex(); // 3 + lastIndex
			mesh_.pushIndex(mesh_.getIndexNumber()-2); // 2 + lastIndex
		}

		// create a cube for each branch. cube has 8 corners/vertices
		// It is not in the middle of the matrix, is placed on top of the x axis 
		// and in the positive portion of z axis
		void pushCube (int heightMult = 1, int widthMult = 1) {
			float n = 1.0f/4.0f;  // 4 is the number of photos in .gif
      float _halfHeight = halfHeight*heightMult;
      float _halfWidth = halfWidth*widthMult;
			// left bottom front
			vec4 v1 =  vec4 ( -_halfWidth, 0.0f, 0.0f, 1.0f) * modelToWorld;
			// right bottom front
			vec4 v2 = vec4 (  _halfWidth, 0.0f, 0.0f, 1.0f) * modelToWorld;
			// right top front
			vec4 v3 =  vec4 (  _halfWidth, 2*_halfHeight, 0.0f, 1.0f ) * modelToWorld;
			// left top front
			vec4 v4 =  vec4 ( -_halfWidth, 2*_halfHeight, 0.0f, 1.0f) * modelToWorld;
			// right bottom back
			vec4 v5 = vec4 ( _halfWidth, 0.0f, -2*_halfWidth, 1.0f) * modelToWorld;
			// right top back
			vec4 v6 = vec4 ( _halfWidth, 2*_halfHeight, -2*_halfWidth, 1.0f) * modelToWorld;
			// left bottom back
			vec4 v7 = vec4 ( -_halfWidth, 0.0f, -2*_halfWidth, 1.0f) * modelToWorld;
			// left top back
			vec4 v8 = vec4 ( -_halfWidth, 2*_halfHeight, -2*_halfWidth, 1.0f) * modelToWorld;
			
			// push vertices and indices into the mesh according to the 8 vertices
			pushIntoMesh( v1, v2, v3, v4, v5, v6, v7, v8);

			// 6 calls, each for each face of cube
			for ( int i = 0; i < 6; i++) {
				mesh_.pushUV( 0.2f, 0.0f);
				mesh_.pushUV( n, 0.0f);
				mesh_.pushUV( n, 0.14f);
				mesh_.pushUV( 0.2f, 0.14f);
			}
			/*
			// 6 calls, each for each face of cube
			// each call has uv coordinates for 4 vertices
			mesh_.pushUVCoordinates1();
			mesh_.pushUVCoordinates1();
			mesh_.pushUVCoordinates1();
			mesh_.pushUVCoordinates1();
			mesh_.pushUVCoordinates1();
			mesh_.pushUVCoordinates1();
			*/
		}

    // Push a cone/cone section to mesh that has:
    // Base circunference radius proportional to r0Mult
    // Top circunference radius proportional to r1mult
    // Height proportional to heightMult
    // Circunference drawn using nPoints
    void pushCone (int r0Mult, int r1Mult, int heightMult, int nPoints = 10) {
      // 4 is the number of photos on the horizontal axis in the .gif
      // - TextureOffset to avoid texture bleeding
      float n = (1.0f/4.0f) - getTextureOffset();

      // Total height of the cone
      float height = 2.0f*halfHeight*heightMult;
      // Total radius of the base circunference
      float r0 = halfWidth*r0Mult;
      // Total radius of the top circunference
      float r1 = halfWidth*r1Mult;
      // Interval between angles used to obtain points
      float angleStep = 2*3.1415926536f/nPoints;
      // Current angle used to obtain points
      float angle = 0.0f;

      // Current values obtained with angle 0
      vec4 v2 = vec4 (0.0f,      0, r0, 1.0f) * modelToWorld;
      vec4 v3 = vec4 (0.0f, height, r1, 1.0f) * modelToWorld;

      for ( int i = 1; i <= nPoints; ++i ) {
        // Increment the angle by the interval
        angle += angleStep;
        // The previous points are going to be the last calculated
        vec4 v1 = v2; // down prev
        vec4 v4 = v3; // up prev
        // The current points are going to be calculated using the angle and radius
        v2 = vec4 ( r0*sinf(angle),      0, r0*cosf(angle), 1.0f ) * modelToWorld; // down current
        v3 = vec4 ( r1*sinf(angle), height, r1*cosf(angle), 1.0f ) * modelToWorld; // up current

        // Create a trapezoid using this points
        pushFrontFace( v1, v2, v3, v4);

        // Push UV for the front face created
        // Not starting from 0.0f in order to avoid texture bleeding via wrapping
        mesh_.pushUV( getTextureOffset(), getTextureOffset());
				mesh_.pushUV( n, getTextureOffset());
				mesh_.pushUV( n, 0.14f);
				mesh_.pushUV( getTextureOffset(), 0.14f);
      }
    }


    void pushFrontFace ( vec4 &v1, vec4 &v2, vec4 &v3, vec4 &v4) {
      // push front face vertices into the mesh			
			mesh_.pushVertex(vec3 (v4.x(), v4.y(), v4.z()));
			mesh_.pushVertex(vec3 (v3.x(), v3.y(), v3.z()));
			mesh_.pushVertex(vec3 (v1.x(), v1.y(), v1.z()));
			mesh_.pushVertex(vec3 (v2.x(), v2.y(), v2.z()));
			// push indices for the front face
			pushIndexForQuad();
    }

		// push vertices and indices that form a cube into the mesh
		void pushIntoMesh ( vec4 &v1, vec4 &v2, vec4 &v3, vec4 &v4, vec4 &v5, vec4 &v6, vec4 &v7, vec4 &v8) {
			// push front face vertices into the mesh			
			mesh_.pushVertex(vec3 (v4.x(), v4.y(), v4.z()));
			mesh_.pushVertex(vec3 (v3.x(), v3.y(), v3.z()));
			mesh_.pushVertex(vec3 (v1.x(), v1.y(), v1.z()));
			mesh_.pushVertex(vec3 (v2.x(), v2.y(), v2.z()));
			// push indices for the front face
			pushIndexForQuad();

			// right face
			mesh_.pushVertex(vec3 (v3.x(), v3.y(), v3.z()));
			mesh_.pushVertex(vec3(v6.x(), v6.y(), v6.z()));
			mesh_.pushVertex(vec3(v2.x(), v2.y(), v2.z()));
			mesh_.pushVertex(vec3(v5.x(), v5.y(), v5.z()));

			pushIndexForQuad();	

			// back face
			// maybe not in the right order!!!!!		
			mesh_.pushVertex(vec3(v6.x(), v6.y(), v6.z()));
			mesh_.pushVertex(vec3(v8.x(), v8.y(), v8.z()));
			mesh_.pushVertex(vec3(v5.x(), v5.y(), v5.z()));
			mesh_.pushVertex(vec3(v7.x(), v7.y(), v7.z()));

			pushIndexForQuad();	

			// left face	
			mesh_.pushVertex(vec3(v8.x(), v8.y(), v8.z()));
			mesh_.pushVertex(vec3 (v4.x(), v4.y(), v4.z()));
			mesh_.pushVertex(vec3(v7.x(), v7.y(), v7.z()));
			mesh_.pushVertex(vec3 (v1.x(), v1.y(), v1.z()));

			pushIndexForQuad();

			// up face	
			mesh_.pushVertex(vec3 (v6.x(), v6.y(), v6.z()));
			mesh_.pushVertex(vec3(v8.x(), v8.y(), v8.z()));
			mesh_.pushVertex(vec3 (v3.x(), v3.y(), v3.z()));
			mesh_.pushVertex(vec3(v4.x(), v4.y(), v4.z()));

			pushIndexForQuad();
			
			// bottom face
			// maybe its clockwise instead of counterclockwise
			mesh_.pushVertex(vec3(v7.x(), v7.y(), v7.z()));
			mesh_.pushVertex(vec3 (v5.x(), v5.y(), v5.z()));
			mesh_.pushVertex(vec3(v1.x(), v1.y(), v1.z()));
			mesh_.pushVertex(vec3 (v2.x(), v2.y(), v2.z()));

			pushIndexForQuad();
		}

		// create leaf shapes
		void pushLeafs () {
			float n = 1.0f/4.0f;  // 4 is the number of photos in .gif
			//modelToWorld.rotateZ(90.0f);
			vec4 v1 =  vec4 ( 0.0f, 0.0f, 0.0f, 1.0f) * modelToWorld;
			vec4 v2 = vec4 (  halfHeight/2, halfHeight/2.0f, 0.0f, 1.0f) * modelToWorld;
			vec4 v3 =  vec4 (  0.0f, halfHeight, 0.0f, 1.0f ) * modelToWorld;
			vec4 v4 =  vec4 ( -halfHeight/2, halfHeight/2.0f, 0.0f, 1.0f) * modelToWorld;
			vec4 v5 = vec4 ( halfHeight/2, halfHeight/2.0f, -2*halfWidth, 1.0f) * modelToWorld;
			vec4 v6 = vec4 ( 0.0f, halfHeight, -2*halfWidth, 1.0f) * modelToWorld;
			vec4 v7 = vec4 ( 0.0f, 0.0f, -2*halfWidth, 1.0f) * modelToWorld;
			vec4 v8 = vec4 ( -halfHeight/2, halfHeight/2.0f, -2*halfWidth, 1.0f) * modelToWorld;
			//modelToWorld.rotateZ(-90.0f);
			pushIntoMesh( v1, v2, v3, v4, v5, v6, v7, v8);

			// 6 calls, each for each face of cube
			for ( int i = 0; i < 6; i++) {
				mesh_.pushUV( n, 0.0f);
				mesh_.pushUV( n + n, 0.0f);
				mesh_.pushUV( n + n, 0.14f);
				mesh_.pushUV( n, 0.14f);
			}
			/*
			// 6 calls, each for each face of cube
			mesh_.pushUVCoordinates2();
			mesh_.pushUVCoordinates2();
			mesh_.pushUVCoordinates2();
			mesh_.pushUVCoordinates2();
			mesh_.pushUVCoordinates2();
			mesh_.pushUVCoordinates2();
			*/

		}

		void meshReady () { 
			modelToWorld.loadIdentity(); // this has to be calles because after the trees the modelToWorld is not the identity

			texture_handle_ = resource_dict::get_texture_handle(GL_RGB, "assets/jungle/terrain.gif");  
		    glActiveTexture(GL_TEXTURE0);
		    glBindTexture(GL_TEXTURE_2D, texture_handle_);
		    // http://www.codeproject.com/Articles/236394/Bi-Cubic-and-Bi-Linear-Interpolation-with-GLSL
		    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			mesh_.setBuffers(); 
		}		

		void pushModelMatrix () { modelToWorldMatrices.push_back(modelToWorld); }
		void popModelMatrix () { modelToWorldMatrices.pop_back(); }
		void setIdentity(mat4t &matrix) { matrix.loadIdentity(); }

		// render mesh
		void render(color_shader &shader, mat4t &cameraToWorld) { 
		  modelToWorld.loadIdentity();
		  mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
		  shader.render(modelToProjection, color);
		  mesh_.render();
		}

		// render mesh with texture shader
		void render(texture_shader &shader, mat4t &cameraToWorld) { 
		  //modelToWorld.loadIdentity();
		  mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
		  //texture_handle_ = resources::get_texture_handle(GL_RGB, "assets/terrain.gif");  
		  //glActiveTexture(GL_TEXTURE0);
		  //glBindTexture(GL_TEXTURE_2D, texture_handle_);
		  // http://www.codeproject.com/Articles/236394/Bi-Cubic-and-Bi-Linear-Interpolation-with-GLSL
		  //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		 // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		  shader.render(modelToProjection, 0);  		
		  mesh_.render();
		}
		
		void reset () {			
			modelToWorld.loadIdentity();
			modelToWorldMatrices.reset();
			mesh_.reset();
		}
	};
}