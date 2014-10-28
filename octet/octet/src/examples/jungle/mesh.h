

namespace octet {
  // mesh holds indices, vertices, uv coordinates
	class Mesh {
		dynarray<float> vertices;
		dynarray<GLuint> indices;
		dynarray<float> uvcoord;
		int index_number;
		GLuint vertexbuffer;
		GLuint indexbuffer;
		GLuint uvbuffer;
    
    static void setVBO(const Mesh& mesh, octet::scene::mesh* octet_mesh) {
      if ((mesh.vertices.size() / 3) * sizeof(octet::scene::mesh::vertex) > octet_mesh->get_vertices()->get_size()) {
        printf("WARNING! Mesh vertices size > Octet Mesh vertices size!");
        return;
      }

      octet::gl_resource::rwlock lock(octet_mesh->get_vertices());
      octet::mesh::vertex* vertex = (octet::scene::mesh::vertex*) lock.u8();

      size_t vertexIndex = 0;
      size_t uvIndex = 0;

      while (vertexIndex < mesh.vertices.size() && uvIndex < mesh.uvcoord.size()) {
        // IMPORTANT Do not apply postfix increment operator in arguments since argument
        //           order evaluation is not specified for compiler implementations
        vertex->pos = vec3p(mesh.vertices[vertexIndex], mesh.vertices[vertexIndex + 1], mesh.vertices[vertexIndex + 2]);
        vertex->uv = vec2p(mesh.uvcoord[uvIndex], mesh.uvcoord[uvIndex + 1]);
        
        // Since normal is undefined in Mesh, we set it to the zero vector
        vertex->normal = vec3p(0.f, 0.f, 0.f);

        vertexIndex += 3;
        uvIndex += 2;

        // Pointer arithmetic
        // Move onto next item location
        ++vertex;
      };

      // IMPORTANT set the number of vertices
      octet_mesh->set_num_vertices(vertexIndex / 3);
    };

    static void setIBO(const Mesh& mesh, octet::scene::mesh* octet_mesh) {
      if (mesh.indices.size() * sizeof(uint32_t) > octet_mesh->get_indices()->get_size()) {
        printf("WARNING! Mesh indices size > Octet Mesh indices size!");
        return;
      }

      octet::gl_resource::rwlock lock(octet_mesh->get_indices());
      uint32_t* index = lock.u32();

      ::memcpy(index, mesh.indices.data(), mesh.indices.size() * sizeof(GLuint));

      //for (size_t i = 0; i < mesh.indices.size(); ++i) {
      //  *index = mesh.indices[i];
      //  ++index;
      //};
      
      // IMPORTANT set the number of indices
      octet_mesh->set_num_indices(mesh.indices.size());
    };
    
    // Given a triangle face, it calculate the normal accordingly
    // NOTE Watch out for clockwise/anticlockwise rotation
    static octet::math::vec3 calculateNormal(const octet::math::vec3& a, const octet::math::vec3& b, const octet::math::vec3& c) {
      octet::math::vec3 v = b - a;//c - a;
      octet::math::vec3 r = c - a;//b - a;

      return vec3(
          v[1] * r[2] - v[2] * r[1],
	        v[2] * r[0] - v[0] * r[2],
	        v[0] * r[1] - v[1] * r[0]
	      );

      // SSE issues with cross product...
      //return (b - a).cross(c - a);
    };

	public :
		Mesh () { }

		void init () {
			index_number = 0;
		}

		void reset () {
			index_number = 0;
			vertices.reset();
			indices.reset();
			uvcoord.reset();
		}

		void pushVertex ( vec3 &vector) {
			vertices.push_back(vector.x());
			vertices.push_back(vector.y());
			vertices.push_back(vector.z());
		}

		void pushIndex ( ) {
			indices.push_back( index_number );
			index_number++;
		}

		void pushIndex ( int n ) {
			indices.push_back( n );
		}
	
		void pushUV ( const octet::vec2& uv ) { 
			pushUV(uv.x(), uv.y());
		}

		void pushUV ( float u_, float v_ ) {
      uvcoord.push_back( u_ );
      uvcoord.push_back( v_ ); 
		}

		dynarray<float> &getVertices () { return vertices; }

		dynarray<GLuint> &getIndices () { return indices; }

		int &getIndexNumber () { return index_number; }

		// set buffers and enable attributes when mesh is ready
		void setBuffers () {
			glEnableVertexAttribArray(attribute_pos);
		    glGenBuffers(1, &vertexbuffer);			
		    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW);  // 3 korifes * sizeof(float)
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

			// http://stackoverflow.com/questions/15041120/crash-at-draw-call-in-nvoglv32-dll-on-new-video-card
      if(!uvcoord.empty()){
			glEnableVertexAttribArray(attribute_uv);
			glGenBuffers(1, &uvbuffer);	
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, uvcoord.size()*sizeof(float), &uvcoord[0], GL_STATIC_DRAW);  // 3 korifes * sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
			}

		    //glEnableVertexAttribArray(attribute_pos);
			//glVertexPointer(3, GL_FLOAT, 0, 0); //sizeof(GLfloat)*3
			glGenBuffers(1, &indexbuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
		}

		void render() {
			//setBuffers (); // this is needed only in the case that we have multiple meshes
			
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

		}

    /// Convert from Mesh to octet::mesh
    /// NOTE This is only temporary. This will be replaced with a better
    ///      version once the concept is better understood.
    static void asMesh(const Mesh& mesh, octet::scene::mesh* octet_mesh) {
      setVBO(mesh, octet_mesh);
      setIBO(mesh, octet_mesh);
    };

    /// Automatically generates normals based on index buffer contents
    static void calculateNormals(octet::scene::mesh* octet_mesh) {
      if (octet_mesh->get_mode() != GL_TRIANGLES) {
        return;
      }

      // Acquire read-write lock on vertex buffer
      octet::gl_resource::rwlock vertexLock(octet_mesh->get_vertices());
      octet::mesh::vertex* vertex = (octet::mesh::vertex*) vertexLock.u8();
      
      // Acquire read-only lock on index buffer
      octet::gl_resource::rolock indexLock(octet_mesh->get_indices());
      const uint32_t* index = indexLock.u32();
      
      // 1st pass sums all normals of neighbouring faces
      octet::math::vec3 normal(0.f);
      for (size_t i = 0; i < octet_mesh->get_num_indices(); i += 3) {
        // Convenient aliases representing each vertex of a triangle face
        octet::scene::mesh::vertex& a = vertex[index[i]];
        octet::scene::mesh::vertex& b = vertex[index[i + 1]];
        octet::scene::mesh::vertex& c = vertex[index[i + 2]];

        // Calculate normal for each triangle face
        // NOTE converter operator of vec3p to vec3 is being issued automatically
        normal = calculateNormal(a.pos, b.pos, c.pos);

        // Update aggregate sum of normals
        // vec3 wrapper due to conversion from vec3p to vec3
        a.normal = octet::math::vec3(a.normal) + normal;
        b.normal = octet::math::vec3(b.normal) + normal;
        c.normal = octet::math::vec3(c.normal) + normal;
      }

      // 2nd pass simply normalizes all normals (average sum of normals)
      for (size_t i = 0; i < octet_mesh->get_num_vertices(); ++i) {
        // vec3 wrapper due to conversion from vec3p to vec3
        vertex[i].normal = octet::math::vec3(vertex[i].normal).normalize();
      }
    };
	};
}