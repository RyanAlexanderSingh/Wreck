////////////////////////////////////////////////////////////////////////////////
//
// Space Monkey Mafia - Procedural Jungle Generation
//
// URL: https://www.doc.gold.ac.uk/~mas01at/proj2013/?page_id=4
//
// Artemis Tsouflidou     [ma303at]
// Brian Gatt             [ma301bg]
// Stefana Ovesia         [ma301so]
// Gustavo Arcanjo Silva  [io301gas]
//
////////////////////////////////////////////////////////////////////////////////

namespace octet {

  // Adapted from invaderers.h
  class sprite {
    // where is our sprite (overkill for a 2D game!)
    mat4t modelToWorld;

    octet::ref<octet::mesh> _mesh;

    // half the width of the sprite
    float halfWidth;

    // half the height of the sprite
    float halfHeight;

    // what texture is on our sprite
    int texture;

    static mat4t createUICamera() {
      mat4t camera(1.f);
      camera.translate(0.f, 0.f, 1.f);
      return camera;
    };

    void initializeMesh() {
      if (!_mesh) {
        _mesh = new mesh(4, 0);
      }

      _mesh->set_mode(GL_TRIANGLE_FAN);
      _mesh->set_index_type(0);
      _mesh->set_num_vertices(4);
      
      octet::gl_resource::rwlock verticesLock(_mesh->get_vertices());

      octet::mesh::vertex* vertices = (octet::mesh::vertex*) verticesLock.u8();

      vertices[0].pos = vec3(-halfWidth, -halfHeight, 0);
      vertices[0].uv = vec2(0, 0);

      vertices[1].pos = vec3(halfWidth, -halfHeight, 0);
      vertices[1].uv = vec2(1, 0);

      vertices[2].pos = vec3(halfWidth,  halfHeight, 0);
      vertices[2].uv = vec2(1, 1);

      vertices[3].pos = vec3(-halfWidth,  halfHeight, 0);
      vertices[3].uv = vec2(0, 1);
    };

  public:
    sprite() :
      modelToWorld(1.0f),
      halfWidth(1.f),
      halfHeight(1.f),
      texture(0) {
    }
    
    void init(image* image, float x = 0.0f, float y = 0.0f, float w = 2.0f, float h = 2.0f) {
      init(image->get_gl_texture(), x, y, w, h);
    }

    void init(int _texture, float x = 0.0f, float y = 0.0f, float w = 2.0f, float h = 2.0f) {
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      halfWidth = w * 0.5f;
      halfHeight = h * 0.5f;
      texture = _texture;

      initializeMesh();
    }

    void render(texture_shader &shader) {
      static mat4t cameraToWorldUI = createUICamera();
      render(shader, cameraToWorldUI);
    }

    void render(texture_shader &shader, mat4t &cameraToWorld) {
      // invisible sprite... used for gameplay.
      if (!texture) return;

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      shader.render(modelToProjection, 0);

      _mesh->render();
    }

    // move the object
    void translate(float x, float y) {
      modelToWorld.translate(x, y, 0);
    }

    // position the object relative to another.
    void set_relative(sprite &rhs, float x, float y) {
      modelToWorld = rhs.modelToWorld;
      modelToWorld.translate(x, y, 0);
    }
  };

  class JungleGenerator {
  private:
    RandomNumberGenerator* _rand;

    // Base Polygon
    Polygon2D _base;

    PerlinNoise _noise;
    float _perlinResolution;

    // Discrete grid division for perlin noise point generation
    octet::vec2 _division;

    PolygonSubdivider _subdivider;
    PolygonBorderGenerator _borders;

    BiomeDistributor _biomes;

	  MeshEngine tree;

    sky_shader* _skyShader;

    //mutable float _minY;
    int _colladaModelCount;

    static const char* const getTextureAtlasPath() {
      return "assets/jungle/terrain.gif";
    };
    
    static const char* const getTextureAtlasInfoPath() {
      return "assets/jungle/terrain.xml";
    };

    octet::resource_dict _resources;
    
    octet::image* getTextureAtlasImage() const {
      // Remove constness. Supposedly safe to do so...
      return const_cast<JungleGenerator*>(this)->_resources.get_image(getTextureAtlasPath());
    };
    
    TextureAtlasInfoResource* getTextureAtlasInfo() const {
      // Remove constness. Supposedly safe to do so...
      return (TextureAtlasInfoResource*) const_cast<JungleGenerator*>(this)->_resources.get_resource(getTextureAtlasInfoPath());
    };
    
    octet::vec2 calculateDivision() const {
      Bounds<octet::vec2> bounds = getBoundingBox();

      return octet::vec2(
        (bounds.max.x() - bounds.min.x()) * _perlinResolution,
        (bounds.max.y() - bounds.min.y()) * _perlinResolution
      );
    };

    void initialise() {
      // TODO Randomize Perlin Noise Parameters
      _noise.SetPersistence(0.6f);
      _noise.SetFrequency(0.1f);
      _noise.SetAmplitude(20.0f);
      _noise.SetOctaves(3);
      _noise.SetRandomSeed(1);
      //_noise.SetRandomSeed((unsigned int) randf(_rand) * 100.f);

      _subdivider.setRandomNumberGenerator(_rand);
    };
    
    void generateBasePolygon() {
      //float length = randf(_rand, 50.f, 250.f);
      //_base = SquareLandGenerator().setLength(length).generate();

      _base = EllipticalLandGenerator().setRandomNumberGenerator(_rand).
                 setMinorRadiusBounds(Bounds<float>(150.0f, 700.f)).
                 setMajorRadiusBounds(Bounds<float>(150.0f, 700.f)).
                 setVertexCountBounds(Bounds<size_t>(60, 120)).
                 generate(); 

      _division = calculateDivision();
    };

    void subdivide() {
      _subdivider.reset(&_base);

      // -- Generate Subdivisions

      _subdivider.setPolygonSelectionStrategy(PolygonSubdivider::GreatestArea).
        setEdgeSelectionStrategy(PolygonSubdivider::LongestEdges).
        setPointSelectionStrategy(PolygonSubdivider::Midpoint).
        //subdivide(5).
        subdivide((unsigned int) randf(_rand, 5, 10)).
        setPolygonSelectionStrategy(PolygonSubdivider::RandomPolygon).
        setPointSelectionStrategy(PolygonSubdivider::NearMidpoint).
        //subdivide(5);
        subdivide((unsigned int) randf(_rand, 5, 10));

      // -- Generate Roads

      _borders.setPolygons(&_subdivider.getSubdivisions());
      _borders.generate(randf(_rand, 0.0f, 1.5f));
    };
    
    void assignBiomes() {
      _biomes.setBasePolygon(&_base);
      _biomes.setDivisions(&_subdivider.getSubdivisions());

      _biomes.distribute();
    };

    const Polygon2D& getPolygon(size_t index) const {
      const Polygon2D& polygon = _subdivider.getSubdivisions()[index];
      const PolygonBorderGenerator::BorderedPolygon& borderedPolygon = _borders.getBorderedPolygon(index);
      
      // In case the borders and the polygon exceed the original area of the polygon...
      // This may happen if the border is too wide for the specified polygon
      return (borderedPolygon.getTotalArea() > polygon.getArea() ? polygon : borderedPolygon.getArea());
    };

    float getPerlinHeight(const octet::vec2& point) const {
      return _noise.GetHeight(point.x()/5.0f, point.y()/5.0f);
    };
    
    octet::math::vec2 bilerp(const char* texName, const octet::math::vec2& uv) const {
      return bilerp(texName, getTextureAtlasInfo()->get(), uv);
    };

    octet::math::vec2 bilerp(const char* texName, const TextureAtlasInfo& atlas, const octet::math::vec2& uv) const {
      Bounds<octet::vec2> texUV = getUV(atlas.getTextureByName(texName), atlas);

      return octet::bilerp(
        texUV.min,
        octet::vec2(texUV.max.x(), texUV.min.y()),
        texUV.max,
        octet::vec2(texUV.min.x(), texUV.max.y()),
        uv
      );
    };

    // TODO Use correct UVs
    octet::vec2 getRoadUV(const Bounds<octet::vec2>& bounds, const octet::vec2& vertex) const {
      octet::vec2 uv(
        (vertex.x() - bounds.min.x()) / (bounds.max.x() - bounds.min.x()),
        (vertex.y() - bounds.min.y()) / (bounds.max.y() - bounds.min.y())
      );

      return bilerp("water.gif", uv);
    };

    // TODO Use biome type
    // TODO Use correct UVs
    octet::vec2 getTerrainUV(const Bounds<octet::vec2>& bounds, const Biome& biome, const octet::vec2& vertex) const {
      octet::vec2 uv(
        (vertex.x() - bounds.min.x()) / (bounds.max.x() - bounds.min.x()),
        (vertex.y() - bounds.min.y()) / (bounds.max.y() - bounds.min.y())
      );
      
      switch (biome.getType()) {
      case Biome::RAINFOREST: {

        return bilerp("grass.gif", uv);

      }
      case Biome::SHRUBLAND:
      case Biome::TAIGA: {

        return bilerp("wood.gif", uv);

      }
      case Biome::BARREN:
      default: {

        return bilerp("rock.gif", uv);

      }
      };

      return octet::vec2(0.f);
    };

    // Add points which will be affected by Perlin Noise height within the provided area polygon
    void addPerlinPoints(const Polygon2D& area, const Bounds<octet::vec2>& bounds, Triangulation& triangulator) const {
      // Calculate the minimum discrete x and y coordinates according to perlin noise resolution
      float x = ceil(bounds.min.x() / _division.x()) * _division.x();
      float y = ceil(bounds.min.y() / _division.y()) * _division.y();

      octet::vec2 gridXY(0.f);

      // Add perlin points at discrete intervals according to perlin noise resolution (and associated grid division)
      for (gridXY.x() = x; gridXY.x() < bounds.max.x(); gridXY.x() += _division.x()) {
        for (gridXY.y() = y; gridXY.y() < bounds.max.y(); gridXY.y() += _division.y()) {
          if (area.isPointInside(gridXY)) {
            triangulator.addPoint(gridXY);
          }
        }
      }
    };

    void applySubdivisionTriangles(const Triangulation& triangulation, int offset, Mesh& mesh, octet::dynarray<octet::vec2>& contour) const {
      const dynarray<vec2>& vertices = triangulation.getVertices();
      const dynarray<Triangulation::Triangle>& triangles = triangulation.getTriangles();
       
      // Push vertices
      for (size_t i = offset; i < vertices.size(); ++i) {
        mesh.pushVertex(vec3(vertices[i].x(), getPerlinHeight(vertices[i]), vertices[i].y()));
      }

      // Maintain a dynamic array of inserted contour indices to avoid duplicates
      dynarray<int> contourIndices;
      // Flag which notes whether an offset (omega) vertex has been seen
      bool omega = false;

      unsigned int baseIndex = mesh.getIndexNumber();
      for (size_t i = 0; i < triangles.size(); ++i) {
        const Triangulation::Triangle& triangle = triangles[i];

        // Reset flag
        omega = false;

        for (size_t j = 0; !omega && j < 3; ++j) {
          // If an index is less than the starting offset, i.e. triangle contains a contour vertex
          if (triangle[j].index < offset) {
            for (size_t k = ((j + 1) % 3); j != k; k = ((k + 1) % 3)) {
              // If the vertex is not an omega vertex and it has not yet been inserted, add contour point
              if (!(triangle[k].index < offset) && contains(contourIndices, triangle[k].index) < 0) {
                contourIndices.push_back(triangle[k].index);
                contour.push_back(*(triangle[k].point));
              }
            }

            omega = true;
          }
        }
        
        for (size_t j = 0; !omega && j < 3; ++j) {
          mesh.pushIndex(baseIndex + triangle[j].index - offset);
        }
      }

      mesh.getIndexNumber() += (vertices.size() - offset);
    };

    void generateOmegaQuad(const Bounds<octet::vec2>& bounds, Triangulation& triangulation) const {
      float width = bounds.max.x() - bounds.min.x();
      float height = bounds.max.y() - bounds.min.y();

      triangulation.addPolygonPoint(vec2(bounds.min.x() - width, bounds.min.y() - height));
      triangulation.addPolygonPoint(vec2(bounds.max.x() + width, bounds.min.y() - height));
      triangulation.addPolygonPoint(vec2(bounds.max.x() + width, bounds.max.y() + height));
      triangulation.addPolygonPoint(vec2(bounds.min.x() - width, bounds.max.y() + height));
    };
    
    void triangulateSubdivisions(Mesh& mesh, Triangulation& triangulator, octet::dynarray<octet::dynarray<octet::vec2> >& contours) const {
      for (size_t i = 0; i < _subdivider.getSubdivisions().size(); ++i) {
        triangulator.reset();

        const Polygon2D& polygon = getPolygon(i);
        Bounds<octet::vec2> bounds = octet::getBoundingBox(polygon);

        // Generate Omega/Universal enclosing quad
        // This avoids sharp edges at edges of subdivision and provides
        // a more smooth transition between roads and other subdivisions
        generateOmegaQuad(bounds, triangulator);
        
        // Specify bounding polygon
        for (size_t j = 0; j < polygon.getVertexCount(); ++j) {
          triangulator.addPoint(polygon[j]);
        }

        // Add perlin height points
        addPerlinPoints(polygon, bounds, triangulator);

        // Apply Delaunay triangulation
        triangulator.triangulate();
      
        // Omega Quad offset
        const int offset = 4;

        // Add new contour container
        contours.push_back(dynarray<octet::vec2>());

        // Apply the triangles to the mesh and keep a record of the contour points
        // Omit the first 4 vertices of the omega polygon
        // NOTE Abusing Triangulation implementation since we know that points
        //      are stored in sequential order of insertion
        applySubdivisionTriangles(triangulator, offset, mesh, contours.back());

        // Order contours to abide to polygon requirements
        PolygonUtil::sortConvexPolygonVertices(contours.back(), polygon.getCentroid());
        
        // Apply UVs in the same order the vertices were inserted
        // Omit the first 4 vertices of the omega polygon
        for (size_t j = offset; j < triangulator.getVertices().size(); ++j) {
          mesh.pushUV(getTerrainUV(bounds, _biomes.getBiome(i), triangulator.getVertices()[j]));
        }
      }
    };
    
    void applyRoadTriangles(const Triangulation& triangulation, Mesh& mesh) const {
      const dynarray<vec2>& vertices = triangulation.getVertices();
      const dynarray<Triangulation::Triangle>& triangles = triangulation.getTriangles();
      
      // Keep a record of the lowest vertex height and the last vertex index
      float minY = FLT_MAX;
      size_t vertexBase = mesh.getVertices().size();

      for (size_t i = 0; i < vertices.size(); ++i) {
        float y = getPerlinHeight(vertices[i]);
        mesh.pushVertex(vec3(vertices[i].x(), y, vertices[i].y()));

        minY = std::min(y, minY);
      }

      //_minY = minY;

      // Apply the lowest vertex height to the base polygon vertices to
      // give the impression that the terrain rises at the periphery
      for (size_t i = 0; i < _base.getVertexCount(); ++i) {
        mesh.getVertices()[vertexBase + (i * 3) + 1] = minY;
      }

      unsigned int baseIndex = mesh.getIndexNumber();
      for (size_t i = 0; i < triangles.size(); ++i) {
        for (size_t j = 0; j < 3; ++j) {
          mesh.pushIndex(baseIndex + triangles[i][j].index);
        }
      }

      mesh.getIndexNumber() += vertices.size();
    };

    void triangulateRoads(Mesh& mesh, Triangulation& triangulator, const octet::dynarray<octet::dynarray<octet::vec2> >& contours) const {
      triangulator.reset();
      
      for (size_t i = 0; i < _base.getVertexCount(); ++i) {
        triangulator.addPolygonPoint(_base[i]);
      }
      
      // Specify area hole of previously defined contours for subdivisions
      for (size_t i = 0; i < contours.size(); ++i) {
        triangulator.addHole(contours[i]);
      }

      // Apply Delaunay triangulation
      triangulator.triangulate();
      
      // Apply the triangles to the mesh
      applyRoadTriangles(triangulator, mesh);
      
      // Apply UVs in the same order the vertices were inserted
      Bounds<octet::vec2> bounds = getBoundingBox();
      for (size_t i = 0; i < triangulator.getVertices().size(); ++i) {
        mesh.pushUV(getRoadUV(bounds, triangulator.getVertices()[i]));
      }
    };

    void triangulate(Mesh& mesh) {
      Triangulation triangulator;
      
      octet::dynarray<octet::dynarray<octet::vec2> > subdivisionContours;
      subdivisionContours.reserve(_subdivider.getSubdivisions().size());

      triangulateSubdivisions(mesh, triangulator, subdivisionContours);
      triangulateRoads(mesh, triangulator, subdivisionContours);
    };
    
    float getYSubdivisionValue(const Polygon2D& polygon, const octet::vec2& vertex) const {
      for (size_t i = 0; i < polygon.getVertexCount(); ++i) {
        LineSegment segment(polygon[i], polygon[polygon.nextVertex(i)]);

        // In case vertices lie exactly on edges of subdivisions, simply place them to 0.f
        if (segment.Intersect(LineSegment(polygon[i], vertex)) == LineSegment::COINCIDENT) {
          return 0.f;
        }
      };
      
      // Perlin Noise values are only allowed on vertices which lie *inside* the subdivision
      return getPerlinHeight(vertex);
    };

	  // values necessary for the trees to be rendered
	  void setValues ( dynarray<int> &treenums, dynarray<int> &landtypes, dynarray<vec3> &pos, const Polygon2D& polygon, int type) {
      dynarray<vec2> randomPoints; // keeps track of all the given random points in  an area
		  // TODO with tree density from Biomes
		  int num = static_cast<int>(floor(polygon.getArea()/2000.0f))+1; // create randomly how many trees will be created
		  treenums.push_back(num);
		  float  y ;  // this is the y value that will be returned from perlin noise
		  vec2 point;
		  for ( int i = 0; i < num; i++ ) {
        getRandomPointInDistance(randomPoints, polygon ); //getRandomPoint(polygon, *_rand); // get a random point in the specified region
        point = randomPoints.back();
			  y = getYSubdivisionValue( polygon, point ); // here I give x, z value
			  pos.push_back( vec3( point.x(), y, point.y() ) ); // push the point that a tree should be rendered
		  }
      randomPoints.reset();
		  landtypes.push_back(type);
	  }
   
/*
    void getRandomPointInDistance ( dynarray<vec2> &points, const Polygon2D& polygon ) {
      static const size_t maxTries = 10;
      vec2 point(0.0f);

      bool push = false;
      for (size_t tryCount = 0; !push && tryCount < maxTries; ++tryCount) {
        point = getRandomPoint(polygon, *_rand);

        push = true;
        for ( size_t i = 0; push && i < points.size(); ++i ) {
          push = (point - points[i]).length() > 0.5f;
        }
      }

      // In case maxTries have been reached, the point is simply
      // pushed even though it may intersect
      points.push_back(point);
    }
*/

    void getRandomPointInDistance ( dynarray<vec2> &points, const Polygon2D& polygon ) {
      vec2 point = getRandomPoint(polygon, *_rand);
      for ( size_t i = 0; i < points.size(); ++i ) {
        if ( abs(point.x()-points[i].x()) < 0.5f || abs(point.y()-points[i].y()) < 0.5f ) {
          getRandomPointInDistance( points, polygon );
          return;  // that was return
        }
      }
      points.push_back(point);
    }
  
    void populateJungle( HandleMesh &mesh, visual_scene* scene ) {
		  dynarray<int> treenums;  // number of trees
		  dynarray<int> landtypes; // type of trees
		  dynarray<vec3> positions; // positions of trees
		  vec2 point;
		
		  for ( unsigned int i = 0; i < _subdivider.getSubdivisions().size(); i++ ) { // iterate over all subdivisions
			  const Polygon2D& polygon = getPolygon(i);
			  const Biome &biome = _biomes.getBiome(i);  // it gives me name of tree
			
			  if (biome.getType() == Biome::RAINFOREST) {
				  // set the number of trees, the type, the positions for each one
				  setValues ( treenums, landtypes, positions, polygon, Biome::RAINFOREST);
				
				  // create this type of tree at the right positions
				  tree.init(treenums, landtypes, mesh, positions, scene);
				
			  }
			  else if ( biome.getType() == Biome::BARREN ) {
				  setValues ( treenums, landtypes, positions, polygon, Biome::BARREN);
				
				  tree.init(treenums, landtypes, mesh, positions, scene);
			  }
			  else if ( biome.getType() == Biome::SHRUBLAND ) {
				  setValues ( treenums, landtypes, positions, polygon, Biome::SHRUBLAND);
				
				  tree.init(treenums, landtypes, mesh, positions, scene);
			  }
			  else if ( biome.getType() == Biome::TAIGA ) {
				  setValues ( treenums, landtypes, positions, polygon, Biome::TAIGA);
				
				  tree.init(treenums, landtypes, mesh, positions, scene);
			  }

			  // reset everything to be ready for the next region
			  treenums.reset();
			  landtypes.reset();
			  positions.reset();
		  }
    };
    
#if defined(DRAW_NORMALS) && defined(_DEBUG)
    void renderNormals(octet::scene::mesh_instance* instance, octet::scene::mesh* mesh) {
      octet::mesh* octet_mesh = new octet::mesh(mesh->get_num_vertices() * 2, mesh->get_num_indices() * 2);
      octet::material* octet_material = new material(vec4(0.0f, 0.0f, 1.0f, 1.0f));
      
      octet_mesh->set_mode(GL_LINES);

      octet::gl_resource::rolock lock(mesh->get_vertices());
      octet::scene::mesh::vertex* originalVertex = (octet::scene::mesh::vertex*) lock.u8();
      
      octet::gl_resource::rwlock vertexLock(octet_mesh->get_vertices());
      octet::scene::mesh::vertex* vertex = (octet::scene::mesh::vertex*) vertexLock.u8();

      octet::gl_resource::rwlock indexLock(octet_mesh->get_indices());
      uint32_t* index = indexLock.u32();

      size_t j = 0;
      for (size_t i = 0; i < mesh->get_num_vertices(); ++i) {
        vertex[j].pos = originalVertex[i].pos;
        index[j] = j;

        ++j;

        vertex[j].pos = octet::math::vec3(originalVertex[i].pos) + (1.f * octet::math::vec3(originalVertex[i].normal));
        index[j] = j;
        
        ++j;
      }

      octet_mesh->set_num_vertices(j);
      octet_mesh->set_num_indices(j);
      
      shader *octet_shader = new bump_shader();
      octet_shader->init();

      instance->set_mesh(octet_mesh);
      instance->set_material(octet_material);
      instance->set_shader(octet_shader);
    };
#endif

  public:
    JungleGenerator(int colladaModelCount = -1) :
      _rand(&LinearCongruential::getInstance()),
      _perlinResolution(1.f / 100.f),
      _colladaModelCount(colladaModelCount) {
      //_minY(0.0f) {
    };

    ~JungleGenerator() {
    };
    
    void init() {
      _resources.set_resource(getTextureAtlasPath(), new image(getTextureAtlasPath()));
      _resources.set_resource(getTextureAtlasInfoPath(), new TextureAtlasInfoResource(getTextureAtlasInfoPath()));
    };

    // Returns the lowest vertex on the perimiter defined by the base polygon
    octet::vec3 getLowestBoundaryVertex() const {
      octet::vec3 result((_base.getVertexCount() > 0 ? FLT_MAX : 0.0f));

      for (size_t i = 0; i < _base.getVertexCount(); ++i) {
        float y = getPerlinHeight(_base[i]);

        if (result.y() > y) {
          result.x() = _base[i].x();
          result.y() = y;
          result.z() = _base[i].y();
        }
      }

      return result;
    };

    
    // Returns the lowest vertex on the perimiter defined by the base polygon
    //float getLowestPeripheryYPosition() const {
    //  return _minY;
    //};

	  // Returns the bounding box of the whole 
    Bounds<octet::vec2> getBoundingBox() const {
      if (_base.getVertexCount() > 0) {
        return octet::getBoundingBox(_base);
      }

      return Bounds<octet::vec2>(vec2(0.f), vec2(0.f));
    };

    float getPerlinNoiseResolution() const {
      return _perlinResolution;
    };

    void setPerlinNoiseResolution(float perlinResolution) {
      _perlinResolution = perlinResolution;
    };

    void setSkyShader(sky_shader* shader) {
      _skyShader = shader;
    };

    RandomNumberGenerator* getRandomNumberGenerator() const {
      return _rand;
    };

    void setRandomNumberGenerator(RandomNumberGenerator* rand) {
      _rand = rand;
    };

    void reset() {
      tree.reset(_colladaModelCount);
    };

    void generate() {
      DEBUG_PRINT("Generating Jungle...\n");
      
      DEBUG_PRINT("Initialising procedural algorithms...\n");
      initialise();

      DEBUG_PRINT("Generating base polygon...\n");
      generateBasePolygon();
      
      DEBUG_PRINT("Subdividing...\n");
      subdivide();
      
      DEBUG_PRINT("Assigning biomes...\n");
      assignBiomes();
    };

    void setCameraPath ( CameraKeyboardHandler &camera ) {
      dynarray<vec2> temp;
      // for each subdivided area get the center and store it to the camera's path
      for ( unsigned int i = 0; i < _subdivider.getSubdivisions().size(); i++ ) { // iterate over all subdivisions
			  const Polygon2D& polygon = getPolygon(i);
        //camera.pushVertexPosition( polygon.getCentroid() );
        temp.push_back( polygon.getCentroid() );
      }
      // having all the points we need to sort them 
      camera.pushVertexPosition( temp[0] );
      for ( size_t i = 0; i < temp.size()-1; ++i ) {
        float min = 1000.0f;
        int minIndex = i+1;
        for ( size_t j = i+1; j < temp.size(); ++j ) {
          // choose the smallest distance between x or z axis
          float minD = abs(temp[i].x() - temp[j].x()) < abs(temp[i].y() - temp[j].y()) ? abs(temp[i].x() - temp[j].x()) : (temp[i].y() - temp[j].y()) ;
          if ( minD < min ) {
            min = minD;
            minIndex = j;
          }         
        }
        // we have the nearest area which needs to be the next node at the dynarray
        // then make it next slot by changing its position 
        if ( minIndex != (i+1) ) {
          vec2 temporary = temp[i+1];
          temp[i+1] = temp[minIndex];
          temp[minIndex] = temporary;
        }
 
        camera.pushVertexPosition( temp[i+1] );
      }
      
    };

    void createSkyBox(visual_scene* scene) {
      Bounds<octet::vec2> bounds = getBoundingBox();
      octet::vec3 center(
        bounds.min.x() + ((bounds.max.x() - bounds.min.x())/2.0f),
        0.0f,
        bounds.min.y() + ((bounds.max.y() - bounds.min.y())/2.0f)
        
      );
      material *blue = new material(vec4(0, 0, 1, 1));
      mesh_sphere *sphere = new mesh_sphere(vec3(0), 740, 4);

      scene_node *skyNode = scene->add_scene_node();
      skyNode->translate(center);
      scene->add_child(skyNode);
      scene->add_mesh_instance(new mesh_instance(skyNode, sphere, blue, _skyShader));
    }

    void render(visual_scene* scene) {
      // Once registered with the scene, these will be
      // encapsulated within a ref container
      scene_node* parent = scene->add_scene_node();
      mesh_instance* instance = new mesh_instance(parent);


      renderTerrain(instance);
      
      scene->add_child(parent);
      scene->add_mesh_instance(instance);

      
      scene_node* parent2 = new scene_node();
      mesh_instance* instance2 = new mesh_instance(parent2);

      renderTrees(instance2, scene);


      scene->add_child(parent2);
      scene->add_mesh_instance(instance2);

      renderSea(scene, 1);
      renderSea(scene,2);

      createSkyBox(scene);
      

#if defined(DRAW_NORMALS) && defined(_DEBUG)

      mesh_instance* normals = new mesh_instance(parent);
      renderNormals(normals, instance->get_mesh());
      scene->add_mesh_instance(normals);
#endif // DRAW_NORMALS && _DEBUG
    };


    void renderTrees(mesh_instance* instance, visual_scene* scene) {
      HandleMesh mesh;
      mesh.init();
      
      DEBUG_PRINT("Populating jungle...\n");
      populateJungle(mesh, scene);

      //render(mesh, scene);
      
      // Assign an octet::mesh which contains enough space to
      // handle all the required vertices and indices
      octet::mesh* octet_mesh = new octet::mesh(
        mesh.getMesh().getVertices().size(),
        mesh.getMesh().getIndices().size()
      );

      Mesh::asMesh(mesh.getMesh(), octet_mesh);
      
      DEBUG_PRINT("Calculating normals...\n");
      Mesh::calculateNormals(octet_mesh);

#if DUMP_MESH
      FILE* f = fopen("./mesh.dump", "w");
      if (f) {
        printf("Dumping mesh...\n");
        octet_mesh->dump(f);
      }
#endif // DUMP_MESH

      octet::material* octet_material = new material( getTextureAtlasImage() );
      shader *octet_shader = new bump_shader();
      octet_shader->init();
      
      instance->set_mesh(octet_mesh);
      instance->set_material(octet_material);
      instance->set_shader(octet_shader);

    };

    void renderTerrain(mesh_instance* instance) {
      HandleMesh mesh;
      mesh.init();
      
      DEBUG_PRINT("Triangulating...\n");
      triangulate(mesh.getMesh());
      
      // Assign an octet::mesh which contains enough space to
      // handle all the required vertices and indices
      octet::mesh* octet_mesh = new octet::mesh(
        mesh.getMesh().getVertices().size(),
        mesh.getMesh().getIndices().size()
      );

      Mesh::asMesh(mesh.getMesh(), octet_mesh);
      
      DEBUG_PRINT("Calculating normals...\n");
      Mesh::calculateNormals(octet_mesh);

#if defined(DUMP_MESH)
      FILE* f = fopen("./mesh.dump", "w");
      if (f) {
        printf("Dumping mesh...\n");
        octet_mesh->dump(f);
      }
#endif // DUMP_MESH

      octet::material* octet_material = new material( getTextureAtlasImage() );

      shader* octet_shader = new terrain_shader();
      octet_shader->init();
      
      instance->set_mesh(octet_mesh);
      instance->set_material(octet_material);
      instance->set_shader(octet_shader);
    };

    void renderSea(visual_scene* scene, int half){
      Bounds<octet::vec2> bounds = getBoundingBox();
      octet::vec3 center(
        bounds.min.x() + ((bounds.max.x() - bounds.min.x())/2.0f),
        0.0f,
        bounds.min.y() + ((bounds.max.y() - bounds.min.y())/2.0f)
      );

      mesh_builder seaMesh;
      seaMesh.init();
      //seaMesh.add_planeXZ(2000.0f, 100, 100, center.x()-1000, center.z()-1000);
      seaMesh.add_planeXZ(1500.0f, 200, 200, center.x()-750, center.z()-750, half);
      //seaMesh.translate(-center.x(), 5.0f, -center.y()); 
      ref<material> waterMat = new material(new image("assets/jungle/water.gif"));
      ref<mesh> mWater = new mesh();
      seaMesh.get_mesh(*mWater);

      shader *waterShd = new water_shader();
      waterShd->init();

      scene_node *seaNode = new scene_node();
      //seaNode->access_nodeToParent().translate(0.0f, getLowestPeripheryYPosition(), 0.0f);
      seaNode->access_nodeToParent().translate(0.0f, getLowestBoundaryVertex().y(), 0.0f);

      //seaNode->translate(center);
      scene->add_child(seaNode);
      scene->add_mesh_instance(new mesh_instance(seaNode, mWater, waterMat, waterShd));
      
    }
  };
  
  class jungle_app : public app {
  private:
    const octet::jungle::CmdLineArgs _args;

    ref<visual_scene> _scene;
    CameraKeyboardHandler _camera;

    InputManager _input;

    texture_shader _textureShader;
    octet::ref<sky_shader> _skyShader;
    
    GLuint terrain_texture[3];

    camera_instance* cameraInstance;
    
    JungleGenerator _generator;
    StateMachine _stateMachine;
    RainManager _rainManager;

    struct {
      StateMachine::StateID start;
      StateMachine::StateID loading;
      StateMachine::StateID jungle;
      StateMachine::StateID end;
    } _states;

    random _random;

    bool _audio;

    class JungleState : public State {
    private:
      jungle_app *_parent;

    protected:
      JungleState(jungle_app* parent) :
        _parent(parent) {
      };

      jungle_app* getParent() const {
        return _parent;
      };

    public:
      ~JungleState() { };
    };

    class StartState : public JungleState {
    public:
      StartState(jungle_app* parent) :
        JungleState(parent) {
      };

      void operator()() {
        getParent()->_stateMachine.setState(getParent()->_states.loading);
      };
    };
    
    class LoadingState : public JungleState {
    private:
      size_t _tick;

      octet::ref<octet::image> _image;
      sprite _sprite;

      void render() {
        int vx = 0;
        int vy = 0;

        getParent()->get_viewport_size(vx, vy);
        glViewport(0, 0, vx, vy);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        _sprite.render(getParent()->_textureShader);
      };

    public:
      LoadingState(jungle_app* parent) :
        JungleState(parent),
        _tick(0),
        _image(new octet::image("assets/jungle/loading.gif")) {
          _sprite.init(_image);
      };

      void onEnter() {
        _tick = 0;
      };

      void operator()() {
        // NOTE this could have been implemented using another internal state machine
        // but for the scope of this concept, it is overkill.

        // Another approach to go about this would be to generate the jungle on a different
        // thread. Using the _tick hack since it is faster and easier to implement.
        // NOTE Tried applying multi-threaded loading but alas, since rendering the loading
        //      screen and generating the jungle both interact with OpenGL (one rendering loading,
        //      the other generating meshes), undefined behaviour was exhibited.

        // On the first tick, simply render the loading screen
        // On the second tick, generate the jungle and switch state
        switch (_tick) {
        case 0: {
          render();
          break;
        }

        default: {
          // Generate the jungle instance
          getParent()->generateJungle();

          getParent()->_stateMachine.setState(getParent()->_states.jungle);
        }
        };

        ++_tick;
      };
    };
    
    class RunningState : public JungleState {
    public:
      RunningState(jungle_app* parent) :
        JungleState(parent) {
      };

      void onEnter() {
        getParent()->setAudio(getParent()->_audio);

        // Reset the camera based on the newly generated jungle
        getParent()->resetCamera();
      };

      void onExit() {
        bool audio = getParent()->_audio;
        getParent()->setAudio(false);
        getParent()->_audio = audio;
      };

      void operator()() {
        getParent()->handleInput();
        getParent()->simulate();
        getParent()->draw();
      };
    };

    class EndState : public JungleState {
    private:
      octet::ref<octet::image> _image;
      sprite _sprite;

      void clearScreen() {
        int vx = 0;
        int vy = 0;

        getParent()->get_viewport_size(vx, vy);
        glViewport(0, 0, vx, vy);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        _sprite.render(getParent()->_textureShader);
      };

    public:
      EndState(jungle_app* parent) :
        JungleState(parent),
        _image(new octet::image("assets/jungle/thankyou.gif")) {
          _sprite.init(_image);
      };

      void operator()() {
        clearScreen();
      };
    };

    void resetScene() {
      if (_scene) {
        _scene->release();
      }

      _scene = new visual_scene();

      _skyShader->resetTime();
    };

    void createDefaultCameraAndLight() {
      Bounds<octet::vec2> bounds = _generator.getBoundingBox();

      float x = bounds.min.x() + ((bounds.max.x() - bounds.min.x()) / 2.0f);
      float z = bounds.min.y() + ((bounds.max.y() - bounds.min.y()) / 2.0f);

      //static const float yOffset = 30.0f;
      
      // Create an empty scene node at the centre of the generated jungle
      scene_node* centre = _scene->add_scene_node();
      centre->access_nodeToParent().translate(x, 0.0f, z);

      // Create a new light source representing the sun
      light_instance* sun = new light_instance();
      sun->set_color(vec4(1.0f));
      sun->set_kind(atom_directional);

      scene_node* sunNode = new scene_node();
      sunNode->access_nodeToParent().loadIdentity();

      // Y location is defined as half the island width + an offset. This allows appropriate rotation without
      // intersecting with the island itself
      sunNode->access_nodeToParent().translate(0.0f, z * 2.0f, 0.0f);
      
      // Sun initially faces downwards looking at the island
      sunNode->access_nodeToParent().rotateX(-90.f);

      // Sun is parented at the centre of the island to allow for correct rotation
      centre->add_child(sunNode);
      sun->set_node(sunNode);

      _scene->add_light_instance(sun);

      // Create default camera instance
      _scene->create_default_camera_and_lights();

      // Attach camera helper to the newly established camera instance
      cameraInstance = _scene->get_camera_instance(0);
      cameraInstance->set_near_plane(0.1f);
      cameraInstance->set_far_plane(10000.0f);

      _camera.setCamera(cameraInstance);

      resetCamera();
    };
	
    void resetCamera() {
      Bounds<octet::vec2> bounds = _generator.getBoundingBox();
      octet::vec2 dim(
        bounds.max.x() - bounds.min.x(),
        bounds.max.y() - bounds.min.y()
      );

	    // Move the camera by half the width and half the height of the bounding box of the jungle
	    // + the base offset of the jungle i.e. jungle x + jungle half width, jungle y + jungle half height
      _camera.getCameraHelper().reset(0.f, 0.f, 0.f);
      _camera.getCamera().translate(bounds.min.x() + (dim.x() / 2.0f), 20.f, bounds.min.y() + (dim.y() / 2.0f));
    };

    void updateLight() {
      static const vec4 gradientStopDay(1.f, 1.f, 1.f, 1.f);
      static const vec4 gradientStopMidday(1.f, 0.4666f, 0.098f, 1.f);
      static const vec4 gradientStopNight(0.f, 0.f, 0.f, 1.f);

      if (_args.getCycle() > 0.0f) {
        // Identify the time in relation to the day-night cycle
        float t = _skyShader->getTime() / _args.getCycle();

        // Extract the fractional part
        int tInt = (int) ::floor(t);
        t = t - tInt;

        vec4 colour = gradientStopMidday;

        // Apply gradient between gradient stops accordingly
        if (t < 0.5) {
          colour = lerp((tInt % 2 == 0 ? gradientStopDay : gradientStopNight), gradientStopMidday, t / 0.5f);
        } else {
          colour = lerp(gradientStopMidday, (tInt % 2 == 0 ? gradientStopNight : gradientStopDay), (t - 0.5f) / 0.5f);
        }

        light_instance* light = _scene->get_light_instance(0);
        light->set_color(colour);

        // Change orientation
        mat4t& lightOrientation = light->get_node()->get_parent()->access_nodeToParent();
        vec4 translation = lightOrientation.w();

        lightOrientation.loadIdentity();
        lightOrientation.w() = translation;
        lightOrientation.rotateZ(180.0f * (_skyShader->getTime() / _args.getCycle()));
      }
    };

    void generateJungle() {
      resetScene();

      _generator.reset();
      _generator.generate();
      _generator.render(_scene);

      createDefaultCameraAndLight();

      _rainManager.reset(_scene, _generator.getBoundingBox(), 70.0f);
      _rainManager.setEnabled(false);

      _generator.setCameraPath( _camera );
    };

    void handleInput() {
      _input.poll();

      _camera.update();

		  if (is_key_down(key_space)) {
			  resetCamera();
		  }

      if (_input.processKey(key_f5)) {
        _stateMachine.setState(_states.loading);
      } else if (_input.processKey(key_esc)) {
        _stateMachine.setState(_states.end);
      } else if (_input.processKey('M')) {
        // Toggle audio
        setAudio(!_audio);
      } else if (_input.processKey('R')) {
        // Toggle rain
        _rainManager.setEnabled(!_rainManager.isEnabled());
      }
    };

    void simulate() {
      updateLight();

      const float deltaTime = 1.0f / 30.0f;
      
      _rainManager.update(deltaTime);
      _scene->update(deltaTime);

      // play monkey sounds randomly
      // TODO make it time dependent instead 
      if (_audio && _random.get(0, 500) == 333) {
        audio_manager::get_instance().play_on_silence("monkeys");
      }
    };

    void draw() {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      
      if (vx > 0.f && vy > 0.f) {
        _scene->begin_render(vx, vy, vec4(0.76f, 0.8f, 0.85f, 1.0f));

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, terrain_texture[0]);
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glGenerateMipmap(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, terrain_texture[1]);
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glGenerateMipmap(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, terrain_texture[2]);
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glActiveTexture(GL_TEXTURE0);

        //glActiveTexture(GL_TEXTURE3);
        //glBindTexture(GL_TEXTURE_2D, terrain_texture[3]);
        ////glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        ////glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        //_scene->render(shadersArray, skyShader, *cameraInstance, (float)vx / vy);
        _scene->render(*cameraInstance, (float)vx / vy, _camera.getFogState());
      }
    };

    void setAudio(bool audio) {
      _audio = audio;

      if (_audio) {
        // Start playing background on loop
        audio_manager::get_instance().start_on_loop("backgroundmusic");
      } else {
        // Stop playing audio
        audio_manager::get_instance().stop_playing("monkeys");
        audio_manager::get_instance().stop_playing("backgroundmusic");
      }
    };

  public:
    jungle_app(int argc, char **argv) :
      app(argc, argv),
      _audio(true) {
      srand((unsigned int) time(NULL));
    };

    jungle_app(int argc, char **argv, const octet::jungle::CmdLineArgs& args) :
      app(argc, argv),
      _args(args),
      _generator(args.getModelCount()), 
      _audio(true) {
      srand((unsigned int) ( args.getSeed() > 0 ? args.getSeed() : time(NULL) ));
    };

    ~jungle_app() {
    };

      // Attach input handler
    void app_init() {
      _skyShader = new sky_shader();
      _skyShader->init();
      _skyShader->setCycleDuration(_args.getCycle());

      _textureShader.init();

      _input.attach(this);

      // Attach camera input handler
      _camera.attach(this);

      // Add sound files to audio_manager
      audio_manager::get_instance().add_sound_to_group("assets/jungle/audio/MUS_DKC.wav", "backgroundmusic");
      audio_manager::get_instance().add_numbered_sounds_to_group("assets/jungle/audio/SFX_DK_monkey_", 1, 6, ".wav", "monkeys");

      terrain_texture[0] = resources::resource_dict::get_texture_handle(GL_RGBA, "assets/jungle/sand.gif");
      terrain_texture[1] = resources::resource_dict::get_texture_handle(GL_RGBA, "assets/jungle/grass3.gif");
      //terrain_texture[2] = resources::resource_dict::get_texture_handle(GL_RGBA, "assets/jungle/gravel2.gif");
      terrain_texture[2] = resources::resource_dict::get_texture_handle(GL_RGBA, "assets/jungle/rock.gif");

      _generator.init();
      _generator.setSkyShader(_skyShader);

      // Register states
      _states.start = _stateMachine.registerState(new StartState(this));
      _states.loading = _stateMachine.registerState(new LoadingState(this));
      _states.jungle = _stateMachine.registerState(new RunningState(this));
      _states.end = _stateMachine.registerState(new EndState(this));

      // Prime state machine with the initial state
      _stateMachine.setState(_states.start);

    };

    void draw_world(int x, int y, int w, int h) {
      // Poll state machine
      _stateMachine.update();
    };
  };
}

#undef DEBUG_PRINT