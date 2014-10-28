namespace octet {

  // --

  /**
   *  Bounds (minimum, maximum) representation
   **/
  template <typename T>
  struct Bounds {
    T min;
    T max;
    
    explicit Bounds(const T& min = T(0)) :
      min(min),
      max(min) {
    };

    Bounds(const T& min, const T& max) :
      min(min),
      max(max) {
    };
  };
  
  // --

  /**
   *  2D Polygon.
   **/
  class Polygon2D {
  public:
    typedef octet::vec2 Vertex;
    typedef size_t VertexIndex;

    class Edge {
    private:
      const Polygon2D* _parent;

      VertexIndex _begin;
      VertexIndex _end;

      friend class Polygon2D;
      
      Edge(const Polygon2D* parent, VertexIndex begin) :
        _parent(parent),
        _begin(begin),
        _end(parent->nextVertex(begin)) {
      };

      Edge(const Polygon2D* parent, VertexIndex begin, VertexIndex end) :
        _parent(parent),
        _begin(begin),
        _end(end) {
      };

    public:
      // Invalid Edge.
      // Valid Edge instances can be acquired via
      // copy-construction or via Polygon2D factory method
      Edge() :
        _parent(NULL),
        _begin(-1),
        _end(-1) {
      };

      ~Edge() {
      };

      const Polygon2D* parent() const {
        return _parent;
      };
      
      const Vertex& first() const {
        return _parent->_vertices[_begin];
      };

      const Vertex& second() const {
        return _parent->_vertices[_end];
      };

      VertexIndex firstIndex() const {
        return _begin;
      };

      VertexIndex secondIndex() const {
        return _end;
      };

      float length() const {
        return ::abs((first() - second()).length());
      };
    };

  private:
    // ASSUMPTION: successive vertices follow each other in either
    //             clockwise or anti-clockwise rotation.
    octet::dynarray<Vertex> _vertices;
    
    octet::dynarray<Vertex>::iterator getIterator(VertexIndex index) {
      octet::dynarray<Vertex>::iterator iter = _vertices.begin();
      while (index > 0) {
        ++iter;
        --index;
      }

      return iter;
    };

    // Reference: http://geomalgorithms.com/a03-_inclusion.html
    float isLeft(const Vertex& P0, const Vertex& P1, const Vertex& P2) const {
      return ( (P1.x() - P0.x()) * (P2.y() - P0.y()) - (P2.x() - P0.x()) * (P1.y() - P0.y()) );
    };

  public:
    Polygon2D() {
    };

    ~Polygon2D() {
    };
    
    Polygon2D& addVertex(float x, float y) {
      return addVertex(Vertex(x, y));
    };

    Polygon2D& addVertex(const Vertex& position) {
      _vertices.push_back(position);
      return *this;
    };
    
    bool addVertex(float x, float y, VertexIndex index) {
      return addVertex(Vertex(x, y), index);
    };

    bool addVertex(const Vertex& position, VertexIndex index) {
      if (index < _vertices.size() + 1) {
        // Minor optimisation which avoids iterator
        if (index == _vertices.size()) {
          addVertex(position);
        } else {
          _vertices.insert(getIterator(index), position);
        }

        return true;
      }

      return false;
    };

    const octet::dynarray<Vertex>& getVertices() const {
      return _vertices;
    };
    
    const Vertex& operator[](VertexIndex index) const {
      return _vertices[index];
    };

    size_t getVertexCount() const {
      return _vertices.size();
    };
    
    VertexIndex previousVertex(VertexIndex index) const {
      return (index - 1 < 0 ? _vertices.size() - 1: index - 1);
    };

    VertexIndex nextVertex(VertexIndex index) const {
      return (index + 1) % _vertices.size();
    };

    Edge getEdge(VertexIndex index) const {
      return Edge(this, index);
    };

    // Reference: http://www.wikihow.com/Calculate-the-Area-of-a-Polygon
    float getArea() const {
      float result = 0.f;

      for (VertexIndex i = 0; i < _vertices.size(); ++i) {
        const Vertex& next = _vertices[nextVertex(i)];
        result += (_vertices[i].x() * next.y()) - (_vertices[i].y() * next.x());
      }

      return result * 0.5f;
    };

    // Reference: http://en.wikipedia.org/wiki/Centroid
    Vertex getCentroid() const {
      Vertex centroid(0.f);

      for (size_t i = 0; i < _vertices.size(); ++i) {
        centroid += _vertices[i];
      }
      centroid = centroid / ((float) _vertices.size());

      return centroid; 
    };

    // Reference: http://geomalgorithms.com/a03-_inclusion.html
    bool isPointInside(const Vertex& point) const {
      int wn = 0;    // the  winding number counter

      // NOTE Removing equals sign in order to avoid listing vertices directly on edges
      //      as being inside the polygon

      // loop through all edges of the polygon
      for (size_t i = 0; i < getVertexCount(); i++) {   // edge from V[i] to  V[i+1]
        if ((*this)[i].y() < point.y()) {          // start y <= P.y
          if ((*this)[nextVertex(i)].y() > point.y()) {     // an upward crossing
            if (isLeft((*this)[i], (*this)[nextVertex(i)], point) > 0) { // P left of  edge
              ++wn;            // have  a valid up intersect
            }
          }
        } else {                        // start y > P.y (no test needed)
          if ((*this)[nextVertex(i)].y() < point.y()) {   // a downward crossing
            if (isLeft((*this)[i], (*this)[nextVertex(i)], point) < 0) { // P right of  edge
              --wn;            // have  a valid down intersect
            }
          }
        }
      }

      return wn != 0;
    };

    octet::string toString() const {
      enum { BUFFER_SIZE = 100 };
      static char buffer[BUFFER_SIZE];

      octet::string polyString;
      polyString += "{";
      
      for (VertexIndex i = 0; i < _vertices.size(); ++i) {
        polyString += _vertices[i].toString(buffer, BUFFER_SIZE);

        if (i < _vertices.size() - 1) {
          polyString += ", ";
        }
      };

      polyString += "}";

      return polyString;
    };
  };

  bool operator==(const Polygon2D::Edge& lhs, const Polygon2D::Edge& rhs) {
    return lhs.parent() == rhs.parent() &&
            lhs.firstIndex() == rhs.firstIndex() &&
            lhs.secondIndex() == rhs.secondIndex();
  };

  bool operator!=(const Polygon2D::Edge& lhs, const Polygon2D::Edge& rhs) {
    return !(lhs == rhs);
  };

  Bounds<octet::vec2> getBoundingBox(const Polygon2D& area) {
    octet::vec2 _min(area[0].x(), area[0].y());
    octet::vec2 _max(_min);
      
    for (size_t i = 1; i < area.getVertexCount(); ++i) {
      if (area[i].x() < _min.x()) {
        _min.x() = area[i].x();
      } else if (area[i].x() > _max.x()) {
        _max.x() = area[i].x();
      }

      if (area[i].y() < _min.y()) {
        _min.y() = area[i].y();
      } else if (area[i].y() > _max.y()) {
        _max.y() = area[i].y();
      }
    }

    return Bounds<octet::vec2>(_min, _max);
  };
  
  // Reference: http://stackoverflow.com/questions/240778/random-points-inside-a-polygon
  octet::vec2 getRandomPoint(const Polygon2D& area, RandomNumberGenerator& rand) {
    //if (area.getVertexCount() < 2) {
    //  return octet::vec2(0.f);
    //} else if (area.getVertexCount() == 2) {
    //  return lerp(area[0], area[1], randf(&rand));
    //} else {
    if (area.getVertexCount() > 2) {
      unsigned int numberOfTriangles = area.getVertexCount() - 2;

      // NOTE Instead of simply retrieving a random triangle, we can use the area of each
      //      sub-triangle and take that value into consideration for correct uniform distribution
      unsigned int triangleIndex = (unsigned int) (randf(&rand) * (numberOfTriangles - 1));

      Polygon2D triangle;
      triangle.addVertex(area[0]).addVertex(area[triangleIndex + 1]).addVertex(area[triangleIndex + 2]);

      float a = randf(&rand);
      float b = randf(&rand);

      Polygon2D::Vertex p = (a * (triangle[1] - triangle[0])) + (b * (triangle[2] - triangle[0]));
      if (!triangle.isPointInside(triangle[0] + p)) {
        p = (triangle[1] - triangle[0]) + (triangle[2] - triangle[0]) - p;
      }

      p = triangle[0] + p;

      // HACK
      if (!triangle.isPointInside(p)) {
        // At this point, simply select a random edge and find a point on it
        unsigned int edgeIndex = (unsigned int) (randf(&rand) * 3.0f);

        Polygon2D::Edge edge = triangle.getEdge(edgeIndex);
        p = lerp(edge.first(), edge.second(), a);
      }

      return p;
    }

    return octet::vec2(0.f);
  };
  
  octet::vec2 getRandomPoint(const Polygon2D& area) {
    return getRandomPoint(area, LinearCongruential::getInstance());
  };

  class PolygonUtil {
  private:
    class ConvexVertexComparePredicate {
    private:
      octet::vec2 centroid;

    public:
      ConvexVertexComparePredicate(const octet::vec2& centroid) :
        centroid(centroid) {
      };

      bool operator()(const octet::vec2& lhs, const octet::vec2& rhs) const {
        return ::atan2(lhs.y() - centroid.y(), lhs.x() - centroid.x()) < ::atan2(rhs.y() - centroid.y(), rhs.x() - centroid.x());
      };
    };

  public:
    // Reference: http://shiffman.net/2011/12/23/night-4-sorting-the-vertices-of-a-polygon/
    // Reference: http://stackoverflow.com/questions/19713092/how-to-order-vertices-in-a-non-convex-polygon-how-to-find-one-of-many-solutions
    static void sortConvexPolygonVertices(octet::dynarray<octet::vec2>& vertices, const octet::vec2& centroid) {
      sort(vertices, ConvexVertexComparePredicate(centroid));
    };
  };

  // --

  class Ellipse2D {
  private:
    float _minorRadius;
    float _majorRadius;

  public:
    explicit Ellipse2D(float minorRadius = 0.f, float majorRadius = 0.f) :
      _minorRadius(minorRadius),
      _majorRadius(majorRadius) {
    };

    Ellipse2D& setMinorRadius(float minorRadius) {
      _minorRadius = minorRadius;
      return *this;
    };

    float getMinorRadius() const {
      return _minorRadius;
    };
    
    Ellipse2D& setMajorRadius(float majorRadius) {
      _majorRadius = majorRadius;
      return *this;
    };
    
    float getMajorRadius() const {
      return _majorRadius;
    };

    octet::vec2 getDistanceFromCentre(float angleInRadians) const {
      float cosAngle = ::cosf(angleInRadians);
      float sinAngle = ::sinf(angleInRadians);

      // Reference: http://en.wikipedia.org/wiki/Ellipse#Polar_form_relative_to_center
      float a = _minorRadius * cosAngle;
      float b = _majorRadius * sinAngle;
      float r = (_majorRadius * _minorRadius) / ::sqrt((a * a) + (b * b));

      return octet::vec2(r * cosAngle, r * sinAngle);
    };
  };
  
  // --

  /**
   *  Abstract interface for polygonal land generators.
   **/
  class LandGenerator {
  public:
    virtual ~LandGenerator() {
    };

    virtual Polygon2D generate() = 0;
  };
  
  class EllipticalLandGenerator : public LandGenerator{
  private:
    octet::RandomNumberGenerator* _rand;
    Bounds<float> _minorRadiusBounds;
    Bounds<float> _majorRadiusBounds;
    Bounds<size_t> _vertexBounds;

  public:
    EllipticalLandGenerator() :
      _rand(&octet::LinearCongruential::getInstance()),
      _minorRadiusBounds(10.f, 15.f),
      _majorRadiusBounds(15.f, 20.f), 
      _vertexBounds(4, 10) {
    };

    ~EllipticalLandGenerator() {
    };
    
    EllipticalLandGenerator& setRandomNumberGenerator(octet::RandomNumberGenerator* randomNumberGenerator) {
      _rand = randomNumberGenerator;
      return *this;
    };
    
    EllipticalLandGenerator& setMinorRadiusBounds(const Bounds<float>& bounds) {
      _minorRadiusBounds = bounds;
      return *this;
    };

    EllipticalLandGenerator& setMajorRadiusBounds(const Bounds<float>& bounds) {
      _majorRadiusBounds = bounds;
      return *this;
    };
    
    EllipticalLandGenerator& setVertexCountBounds(const Bounds<size_t>& bounds) {
      _vertexBounds = bounds;
      return *this;
    };

    Polygon2D generate() {
      float minorRadius = octet::randf(_rand, _minorRadiusBounds.min, _minorRadiusBounds.max); 
      float majorRadius = octet::randf(_rand, _majorRadiusBounds.min, _majorRadiusBounds.max); 
      Ellipse2D ellipse(minorRadius, majorRadius);

      Polygon2D polygon;

      size_t vertices = (size_t) octet::randf(_rand, (float) _vertexBounds.min, (float) _vertexBounds.max);
      
      float angle = 0.f;
      float angleDivision = 2.f * pi() / ((float) vertices);

      // Offset to avoid having vertices in the negative quadrants i.e (-x,+y), (+x,-y), (-x,-y)
      octet::vec2 offset(majorRadius, minorRadius);

      for (; vertices != 0; --vertices, angle += angleDivision) {
        polygon.addVertex( ellipse.getDistanceFromCentre(octet::randf(_rand, angle, angle + angleDivision)) + offset );
      }

      return polygon;
    };
  };

  class SquareLandGenerator : public LandGenerator {
  private:
    float _length;
    octet::RandomNumberGenerator* _rand;

  public:
    SquareLandGenerator() :
      _length(10.f),
      _rand(&octet::LinearCongruential::getInstance()) {
    };

    ~SquareLandGenerator() {
    };
    
    float getLength() const {
      return _length;
    };

    SquareLandGenerator& setLength(float length) {
      _length = length;
      return *this;
    };

    SquareLandGenerator& setRandomNumberGenerator(octet::RandomNumberGenerator* randomNumberGenerator) {
      _rand = randomNumberGenerator;
      return *this;
    };

    Polygon2D generate() {
      return Polygon2D().addVertex(0.f, 0.f).
                         addVertex(_length, 0.f).
                         addVertex(_length, _length).
                         addVertex(0.f, _length);
    };
  };

  // --

  class PolygonSubdivider {
  public:
    enum PolygonSelectionStrategy { RandomPolygon = 0, GreatestAverageEdgeLength, GreatestArea, RandomPolygonSelectionStrategy };
    enum EdgeSelectionStrategy { RandomEdge = 0, LongestEdges, RandomEdgeSelectionStrategy };
    enum PointSelectionStrategy { RandomPoint = 0, Midpoint, NearMidpoint, RandomPointSelectionStrategy };
  
/*
    typedef std::unary_function<const octet::dynarray<Polygon2D>&, Polygon2D::VertexIndex> IPolygonSelectionStrategyDef;
    typedef std::unary_function<const Polygon2D&, Polygon2D::Edge> IEdgeSelectionStrategy;
    typedef std::unary_function<const Polygon2D::Edge&, Polygon2D::Vertex> IPointSelectionStrategy;
*/
  private:
    const Polygon2D* _polygon;
    octet::dynarray<Polygon2D> _subdivisions;

    PolygonSelectionStrategy _polygonSelectionStrategy;
    EdgeSelectionStrategy _edgeSelectionStrategy;
    PointSelectionStrategy _pointSelectionStrategy;

    octet::RandomNumberGenerator* _rand;
    
    Polygon2D::VertexIndex getRandomPolygon() const {
      return ((*_rand)() % _subdivisions.size());
    };
    
    Polygon2D::VertexIndex getGreatestAverageEdgeLengthPolygon() const {
      Polygon2D::VertexIndex index = 0;
      float maxAverageEdgeLength = 0;

      float avgEdgeLength = 0.f;
      for (size_t i = 0; i < _subdivisions.size(); ++i) {
        // Reset average edge length
        avgEdgeLength = 0.f;

        // Calculate summation
        for (size_t vertex = 0; vertex < _subdivisions[i].getVertexCount(); ++vertex) {
          avgEdgeLength += ::abs( (_subdivisions[i][vertex] - _subdivisions[i][_subdivisions[i].nextVertex(vertex)]).length() );
        };

        // Calculate average mean
        avgEdgeLength /= (float) _subdivisions[i].getVertexCount();

        // Identify maximum
        if (avgEdgeLength > maxAverageEdgeLength) {
          maxAverageEdgeLength = avgEdgeLength;
          index = i;
        }
      };

      return index;
    };
    
    Polygon2D::VertexIndex getGreatestAreaPolygon() const {
      Polygon2D::VertexIndex index = 0;
      float maxArea = 0.f;

      float area = 0.f;
      for (size_t i = 0; i < _subdivisions.size(); ++i) {
        area = _subdivisions[i].getArea();

        if (area > maxArea) {
          maxArea = area;
          index = i;
        }
      };

      return index;
    };

    Polygon2D::VertexIndex applySelectPolygon() const {
      return applySelectPolygon(_polygonSelectionStrategy);
    }

    Polygon2D::VertexIndex applySelectPolygon(PolygonSelectionStrategy strategy) const {
      if (strategy == RandomPolygonSelectionStrategy) {
        strategy = (PolygonSelectionStrategy) ((*_rand)() % (RandomPolygonSelectionStrategy));
      }

      switch (strategy) {
      case GreatestAverageEdgeLength:
        return getGreatestAverageEdgeLengthPolygon();

      case GreatestArea:
        return getGreatestAreaPolygon();

      case RandomPolygon:
      default:
         return getRandomPolygon();
      };
    };

    Polygon2D::VertexIndex selectPolygon() const {
      return (_subdivisions.size() > 1 ? applySelectPolygon() : 0);
    };
    
    typedef std::pair<Polygon2D::Edge, Polygon2D::Edge> EdgePair;
    EdgePair getRandomEdgePair(const Polygon2D& polygon) const {
      Polygon2D::VertexIndex vertexA = (*_rand)() % polygon.getVertexCount();
      
      Polygon2D::VertexIndex vertexB = vertexA;
      while (vertexB == vertexA) {
        vertexB = (*_rand)() % polygon.getVertexCount();
      }

      return std::make_pair(polygon.getEdge(vertexA), polygon.getEdge(vertexB));
    };

    EdgePair getLongestEdgePair(const Polygon2D& polygon) const {
      Polygon2D::VertexIndex vertexA = 0;
      float edgeALength = 0;

      Polygon2D::VertexIndex vertexB = 1;
      float edgeBLength = 0;

      float length = 0;
      for (size_t i = 0; i < polygon.getVertexCount(); ++i) {
        length = ::abs( (polygon[i] - polygon[polygon.nextVertex(i)]).length() );
        
        if (length > edgeALength) {
          vertexA = i;
          edgeALength = length;
        } else if (length > edgeBLength) {
          vertexB = i;
          edgeBLength = length;
        }
      };
      
      return std::make_pair(polygon.getEdge(vertexA), polygon.getEdge(vertexB));
    };

    EdgePair selectEdges(const Polygon2D& polygon) {
      return selectEdges(polygon, _edgeSelectionStrategy);
    };

    EdgePair selectEdges(const Polygon2D& polygon, EdgeSelectionStrategy strategy) {
      if (strategy == RandomEdgeSelectionStrategy) {
        strategy = (EdgeSelectionStrategy) ((*_rand)() % (RandomEdgeSelectionStrategy));
      }

      switch (strategy) {
      case LongestEdges:
        return getLongestEdgePair(polygon);

      case RandomEdge:
      default:
        return getRandomEdgePair(polygon);
      };
    };
    
    float getDivisionRatio() {
      return getDivisionRatio(_pointSelectionStrategy);
    };

    float getDivisionRatio(PointSelectionStrategy strategy) {
      if (strategy == RandomPointSelectionStrategy) {
        strategy = (PointSelectionStrategy) ((*_rand)() % (RandomPointSelectionStrategy));
      }

      switch (strategy) {
      case Midpoint: return 0.5f;

      case NearMidpoint:
        return 0.5f + (octet::randf(_rand) * 0.5f - 0.25f);

      case RandomPoint:
      default:
        return octet::randf(_rand);
      };
    };

    Polygon2D::Vertex getSubdivisionVertex(const Polygon2D::Edge& edge) {
      return lerp(edge.first(), edge.second(), getDivisionRatio());
    };

    Polygon2D makeSubPolygon(const Polygon2D& polygon, const Polygon2D::Edge& edgeA, const Polygon2D::Vertex& vertexA, const Polygon2D::Edge& edgeB, const Polygon2D::Vertex& vertexB) {
      Polygon2D subPoly = Polygon2D().addVertex(vertexA);

      Polygon2D::VertexIndex index = edgeA.firstIndex();
      while (index != edgeB.firstIndex()) {
        index = polygon.nextVertex(index);
        subPoly.addVertex(polygon[index]);
      }

      return subPoly.addVertex(vertexB);
    };

    typedef std::pair<Polygon2D, Polygon2D> Subdivision;
    Subdivision subdivide(const Polygon2D& polygon, const Polygon2D::Edge& edgeA, const Polygon2D::Edge& edgeB) {
      Polygon2D::Vertex randA = getSubdivisionVertex(edgeA);
      Polygon2D::Vertex randB = getSubdivisionVertex(edgeB);
      
      return std::make_pair(
        makeSubPolygon(polygon, edgeA, randA, edgeB, randB),
        makeSubPolygon(polygon, edgeB, randB, edgeA, randA)
      );
    };

  public:
    PolygonSubdivider() :
      _polygon(NULL),
      _polygonSelectionStrategy(RandomPolygon),
      _edgeSelectionStrategy(RandomEdge),
      _pointSelectionStrategy(RandomPoint),
      _rand(&octet::LinearCongruential::getInstance()) {
        reset();
    };

    explicit PolygonSubdivider(const Polygon2D* polygon) :
      _polygon(polygon),
      _polygonSelectionStrategy(RandomPolygon),
      _edgeSelectionStrategy(RandomEdge),
      _pointSelectionStrategy(RandomPoint),
      _rand(&octet::LinearCongruential::getInstance()) {
        reset();
    };

    ~PolygonSubdivider() {
    };

    void reset() {
      _subdivisions.reset();
      
      // Initially start the list with the original polygon
      if (_polygon != NULL) {
        _subdivisions.push_back(*_polygon);
      }
    };

    void reset(const Polygon2D* polygon) {
      _polygon = polygon;
      reset();
    };
    
    PolygonSubdivider& setRandomNumberGenerator(octet::RandomNumberGenerator* randomNumberGenerator) {
      _rand = randomNumberGenerator;
      return *this;
    };

    PolygonSubdivider& setPolygonSelectionStrategy(PolygonSelectionStrategy strategy) {
      _polygonSelectionStrategy = strategy;
      return *this;
    };

    PolygonSubdivider& setEdgeSelectionStrategy(EdgeSelectionStrategy strategy) {
      _edgeSelectionStrategy = strategy;
      return *this;
    };
    
    PolygonSubdivider& setPointSelectionStrategy(PointSelectionStrategy strategy) {
      _pointSelectionStrategy = strategy;
      return *this;
    };
 
    /**
     *  Execute the subdivision algorithm based on the currently available subdivisions and strategies chosen.
     *
     *  @param subdivisionCount the number of subdivision iterations to perform
     *  @return 'this'
     *
     *  Reference: http://blog.soulwire.co.uk/laboratory/flash/recursive-polygon-subdivision
     **/
    PolygonSubdivider& subdivide(size_t subdivisionCount = 1) {
      for (; subdivisionCount != 0; --subdivisionCount) {
        Polygon2D::VertexIndex toSubdivideIndex = selectPolygon();
        const Polygon2D& toSubdivide = _subdivisions[toSubdivideIndex];

        DEBUG_PRINT("Subdividing: [%s]\n", toSubdivide.toString().c_str());

        // Retrieve 2 distinct edges
        EdgePair edges = selectEdges(toSubdivide);

        // Perform subdivision
        Subdivision subdivision = subdivide(toSubdivide, edges.first, edges.second);
        
        // Remove the original polygon
        _subdivisions.erase(toSubdivideIndex);
        
        DEBUG_PRINT("Inserting: [%s]\n", subdivision.first.toString().c_str());
        DEBUG_PRINT("Inserting: [%s]\n\n", subdivision.second.toString().c_str());

        // Add the new subdivisions to the subdivision list
        _subdivisions.insert(_subdivisions.begin(), subdivision.first);
        _subdivisions.insert(_subdivisions.begin(), subdivision.second);
      };

      return *this;
    };
    
    /**
     *  Retrieve the processed subdivions.
     *  @return the processed subdivisions
     */
    const octet::dynarray<Polygon2D>& getSubdivisions() const {
      return _subdivisions;
    };

    /**
     *  Retrieve the processed subdivions. The state of this object
     *  is reset.
     *  @param container the container which will host the results of subdivision
     **/
    void getSubdivisions(octet::dynarray<Polygon2D>& container) {
      _subdivisions.swap(container);
      _subdivisions.reset();
    };
  };

  // --

  class NeighbourGraph {
  private:
    const octet::dynarray<Polygon2D>* _polygons;
    octet::hash_map<unsigned int, octet::dynarray<unsigned int> > _neighbours;
    
    const Polygon2D& getPolygon(size_t index) const {
      return _polygons->operator[](index);
    };

    bool isRegistered(unsigned int polyA, unsigned int polyB) const {
      int index = _neighbours.get_index(polyA);
      if (index != -1) {
        const octet::dynarray<unsigned int>& neighbours = _neighbours.get_value(index);
        for (size_t i = 0; i < neighbours.size(); ++i) {
          if (neighbours[i] == polyB) {
            return true;
          }
        }
      }

      return false;
    }
    
    static float getEpsilon() {
      return 0.0001f;
    };

    // Reference: http://stackoverflow.com/questions/17333/most-effective-way-for-float-and-double-comparison
    // Reference: http://realtimecollisiondetection.net/blog/?p=89
    static bool equal(float a, float b, float epsilon) {
      //return a == b;
      return ::fabs(a - b) < epsilon;
    };
    
    static bool equal(float a, float b) {
      return equal(a, b, getEpsilon());
    };

    static float getGradient(const Polygon2D::Edge& edge) {
      return ((edge.first().y() - edge.second().y()) / (edge.first().x() - edge.second().x()));
    };

    static float getIntercept(const Polygon2D::Edge& edge) {
      return getIntercept(edge, getGradient(edge));
    };
    
    static float getIntercept(const Polygon2D::Edge& edge, float gradient) {
      return edge.first().y() - (gradient * edge.first().x());
    };

    bool areCollinear(const Polygon2D::Edge& edgeA, const Polygon2D::Edge& edgeB) const {
      /*
        octet::vec2 directionA = (edgeA.second() - edgeA.first()).normalize();
        octet::vec2 directionB = (edgeB.second() - edgeB.first()).normalize();

        return equal(directionA.dot(directionB), 1.f);
      */
      float gradientA = getGradient(edgeA);
      float gradientB = getGradient(edgeB);

      return equal(gradientA, gradientB) && equal(getIntercept(edgeA, gradientA), getIntercept(edgeB, gradientB));
    };

    bool isWithin(const Polygon2D::Vertex& a, const Polygon2D::Vertex& b, const Polygon2D::Vertex& c) const {
      float x1 = std::max(b.x(), c.x());
      float x2 = std::min(b.x(), c.x());

      float y1 = std::max(b.y(), c.y());
      float y2 = std::min(b.y(), c.y());

      return (x1 > a.x() || equal(x1, a.x())) &&
              (a.x() > x2 || equal(x2, a.x())) &&
              (y1 > a.y() || equal(y1, a.y())) &&
              (a.y() > y2 || equal(y2, a.y()));
    };

    bool isWithin(const Polygon2D::Edge& edgeA, const Polygon2D::Edge& edgeB) const {
      //return (edgeA.first() + edgeB.first()) + (edgeB.first() + edgeB.second()) + (edgeB.second() + edgeA.second()) == (edgeA.second() - edgeA.first());
      return isWithin(edgeA.first(), edgeB.first(), edgeB.second()) && isWithin(edgeA.second(), edgeB.first(), edgeB.second());
    };

    bool isConnectingEdge(const Polygon2D& polygon, const Polygon2D::Edge& edge, Polygon2D::Edge* connectingEdge = NULL) const {
      for (Polygon2D::VertexIndex index = 0; index < polygon.getVertexCount(); ++index) {
        Polygon2D::Edge otherEdge = polygon.getEdge(index);
        //if (isWithin(edge, otherEdge) || isWithin(otherEdge, edge)) {
        if (areCollinear(otherEdge, edge) && (isWithin(edge, otherEdge) || isWithin(otherEdge, edge))) {
          if (connectingEdge != NULL) {
            *connectingEdge = otherEdge;
          }

          return true;
        }
      }

      return false;
    };

    bool isNeighbour(const Polygon2D& polyA, const Polygon2D& polyB) const {
      for (Polygon2D::VertexIndex index = 0; index < polyA.getVertexCount(); ++index) {
        if (isConnectingEdge(polyB, polyA.getEdge(index))) {
          return true;
        }
      }

      return false;
    };

    void map(size_t polygonIndex) {
      for (size_t i = 0; i < _polygons->size(); ++i) {
        if (i != polygonIndex && !isRegistered(i, polygonIndex) && isNeighbour(getPolygon(polygonIndex), getPolygon(i))) {
          _neighbours[polygonIndex].push_back(i);
          _neighbours[i].push_back(polygonIndex);
        }
      }
    };

    int getIndex(const Polygon2D& polygon) const {
      if (_polygons != NULL) {
        for (size_t i = 0; i < _polygons->size(); ++i) {
          if (&(getPolygon(i)) == &polygon) {
            return i;
          }
        }
      }

      return -1;
    };

  public:
    NeighbourGraph() :
      _polygons(NULL) {
    };
    void setPolygons(const octet::dynarray<Polygon2D>* polygons) {
      _polygons = polygons;
    };

    const octet::dynarray<Polygon2D>* getPolygons() const {
      return _polygons;
    };

    void reset() {
      _neighbours.clear();
    };

    void reset(const octet::dynarray<Polygon2D>* polygons) {
      setPolygons(polygons);
      reset();
    };

    /**
     *  Maps the neighbours of the polygons.
     **/
    void map() {
      if (_polygons != NULL) {
        for (size_t i = 0; i < _polygons->size(); ++i) {
          map(i);
        }
      }
    };

    /**
     *  @param polygon the polygon to which its neighbours are to be retrieved
     *  @return the list of indices which are neighbours to the requested polygons or
     *          NULL if the polygon cannot be mapped
     **/
    const octet::dynarray<unsigned int>* getNeighbours(const Polygon2D& polygon) const {
      int index = getIndex(polygon);
      return index == -1 ? NULL : getNeighbours(index);
    };
    
    /**
     *  @param polygonIndex the index of the polygon to which its neighbours are to be retrieved
     *  @return the list of indices which are neighbours to the requested polygons or
     *          NULL if the polygon cannot be mapped
     **/
    const octet::dynarray<unsigned int>* getNeighbours(unsigned int polygonIndex) const {
      int index = _neighbours.get_index(polygonIndex);
      return index == -1 ? NULL : &_neighbours.get_value(index);
    };
    
    /**
     *  @see getConnectingEdges(unsigned int, unsigned int, octet::dynarray<std::pair<Polygon2D::Edge, Polygon2D::Edge> >&);
     **/
    void getConnectingEdges(const Polygon2D& polygonA, const Polygon2D& polygonB, octet::dynarray<std::pair<Polygon2D::Edge, Polygon2D::Edge> >& results) const {
      int indexA = getIndex(polygonA);
      int indexB = getIndex(polygonB);

      if (indexA != -1 && indexB != -1) {
        getConnectingEdges(indexA, indexB, results);
      }
    };
    
    /**
     *  @param polygonAIndex the index of the first polygon
     *  @param polygonBIndex the index of the second polygon
     *  @param results the container which will host the pair of edges which connect polygonA to polygonB
     **/
    void getConnectingEdges(unsigned int polygonAIndex, unsigned int polygonBIndex, octet::dynarray<std::pair<Polygon2D::Edge, Polygon2D::Edge> >& results) const {
      int indexA = _neighbours.get_index(polygonAIndex);
      int indexB = _neighbours.get_index(polygonBIndex);
      
      Polygon2D::Edge edgeB;

      // NOTE no need to check isRegistered(indexB, indexA) since the graph is undirected
      if (indexA != -1 && indexB != -1 && isRegistered(indexA, polygonBIndex)) {
        const Polygon2D& polygonA = getPolygon(polygonAIndex);
        const Polygon2D& polygonB = getPolygon(polygonBIndex);

        for (Polygon2D::VertexIndex index = 0; index < polygonA.getVertexCount(); ++index) {
          Polygon2D::Edge edgeA = polygonA.getEdge(index);
          if (isConnectingEdge(polygonB, edgeA, &edgeB)) {
            results.push_back(std::make_pair(edgeA, edgeB));
          }
        }
      }
    };
  };

  // --

  class Biome {
  public:
    enum BiomeType {
      // Height 2
      BARREN,

      // Height 1
      TAIGA,
      SHRUBLAND,

      // Height 0
      RAINFOREST
    };

  private:
    BiomeType _type;

    float _averageHeight;
    float _averageMoisture;
    
    float _treeDensity;

  public:
    Biome() :
      _type(BARREN),
      _averageHeight(0.f),
      _averageMoisture(0.f),
      _treeDensity(0.f) {
    };

    void setAverageHeight(float averageHeight) {
      _averageHeight = averageHeight;
    };

    float getAverageHeight() const {
      return _averageHeight;
    };
    
    void setAverageMoisture(float averageMoisture) {
      _averageMoisture = averageMoisture;
    };

    float getAverageMoisture() const {
      return _averageMoisture;
    };
    
    void setTreeDensity(float treeDensity) {
      _treeDensity = treeDensity;
    };

    float getTreeDensity() const {
      return _treeDensity;
    };

    void setType(BiomeType type) {
      _type = type;
    };

    BiomeType getType() const {
      return _type;
    };
  };

  // --

  class BiomeDistributor {
  private:
    const octet::dynarray<Polygon2D>* _divisions;
    const Polygon2D* _basePolygon;

    const octet::RandomNumberGenerator* _rand;

    octet::dynarray<Biome> _biomes;
    
    float _landSpan;
     
    void calculateLandSpan() {
      _landSpan = -1.f;

      if (_basePolygon != NULL) {
        Polygon2D::Vertex centroid = _basePolygon->getCentroid();

        for (Polygon2D::VertexIndex i = 0; i < _basePolygon->getVertexCount(); ++i) {
          _landSpan = std::max(_landSpan, ::abs( (_basePolygon->getVertices()[i] - centroid).length() ));
        }
      }
    };

    // TODO

    void setAverageHeight(float distance, Biome& biome) const {
      biome.setAverageHeight((_landSpan - distance) / _landSpan);
    };

    void setAverageMoisture(float distance, Biome& biome) const {
      // TODO consider nearby rivers
      biome.setAverageMoisture((_landSpan - distance) / _landSpan);
    };

    void setBiomeType(float distance, Biome& biome) const {
      if (biome.getAverageHeight() < 0.5f) {
        biome.setType(Biome::RAINFOREST);
      } else if (biome.getAverageHeight() < 0.6f) {
        biome.setType(Biome::TAIGA);
      } else if (biome.getAverageHeight() < 0.75f) {
        biome.setType(Biome::SHRUBLAND);
      } else {
        biome.setType(Biome::BARREN);
      }
    };

    void setTreeDensity(float distance, Biome& biome) const {
      switch (biome.getType()) {
      case Biome::BARREN    : biome.setTreeDensity(0.f); break;
      case Biome::SHRUBLAND : biome.setTreeDensity(0.05f); break;
      case Biome::TAIGA     : biome.setTreeDensity(0.2f); break;
      case Biome::RAINFOREST: biome.setTreeDensity(0.8f); break;
      };
    };

    float getDistanceFromEdgeOfWorld(const Polygon2D& polygon) const {
      return _landSpan - ::abs( (polygon.getCentroid() - _basePolygon->getCentroid()).length() );
    };

    Biome generateBiome(const Polygon2D& polygon) const {
      Biome biome;

      float distance = getDistanceFromEdgeOfWorld(polygon);

      setAverageHeight(distance, biome);
      setAverageMoisture(distance, biome);
      setBiomeType(distance, biome);
      setTreeDensity(distance, biome);

      return biome;
    };

  public:
    BiomeDistributor() :
      _divisions(NULL),
      _basePolygon(NULL),
      _rand(&octet::LinearCongruential::getInstance()),
      _landSpan(-1.f) {
    };

    void reset() {
      _biomes.reset();
    }

    void setBasePolygon(const Polygon2D* basePolygon) {
      _basePolygon = basePolygon;
      calculateLandSpan();
    };

    void setDivisions(const octet::dynarray<Polygon2D>* divisions) {
      _divisions = divisions;
    };
    
    void setRandomNumberGenerator(const octet::RandomNumberGenerator& random) {
      _rand = &random;
    };

    const Biome& getBiome(unsigned int subdivisionIndex) const {
      return _biomes[subdivisionIndex];
    };

    void distribute() {
      if (_basePolygon != NULL && _divisions != NULL) {
        reset();

        for (size_t i = 0; i < _divisions->size(); ++i) {          
          _biomes.push_back( generateBiome(_divisions->operator[](i)) );
        }
      }
    };

  };

  // --

  // Reference: http://paulbourke.net/geometry/pointlineplane/
  // Reference: http://paulbourke.net/geometry/pointlineplane/example.cpp
  //
  // -- NOTE --
  // This piece of code is copied from the reference above. Personal attempts
  // at writing something similar representing equations of lines led to multiple
  // issues with infinity and indeterminate handling.
  class LineSegment {
  public:
    typedef octet::vec2 Vector;

  private:
    Vector begin_;
    Vector end_;

  public:
    LineSegment()
        : begin_(0.f), end_(0.f) {}

    LineSegment(const Vector& begin, const Vector& end)
        : begin_(begin), end_(end) {}

    enum IntersectResult { PARALLEL, COINCIDENT, NOT_INTERESECTING, INTERESECTING };
    
    IntersectResult Intersect(const LineSegment& other_line, Vector& intersection) const {
      return Intersect(other_line, &intersection);
    }

    IntersectResult Intersect(const LineSegment& other_line, Vector* intersection = NULL) const {
      // NOTE: This algorithm makes use of the vector form of the equation of a line
      // Reference: http://www.vitutor.com/geometry/line/vector_equation.html

      float denom = ((other_line.end_.y() - other_line.begin_.y())*(end_.x() - begin_.x())) -
                    ((other_line.end_.x() - other_line.begin_.x())*(end_.y() - begin_.y()));

      float nume_a = ((other_line.end_.x() - other_line.begin_.x())*(begin_.y() - other_line.begin_.y())) -
                      ((other_line.end_.y() - other_line.begin_.y())*(begin_.x() - other_line.begin_.x()));

      float nume_b = ((end_.x() - begin_.x())*(begin_.y() - other_line.begin_.y())) -
                      ((end_.y() - begin_.y())*(begin_.x() - other_line.begin_.x()));

      if(denom == 0.0f)
      {
          if(nume_a == 0.0f && nume_b == 0.0f)
          {
              return COINCIDENT;
          }
          return PARALLEL;
      }

      float ua = nume_a / denom;
      float ub = nume_b / denom;

      if(ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f)
      {
          if (intersection != NULL) {
            // Get the intersection point.
            intersection->x() = begin_.x() + ua*(end_.x() - begin_.x());
            intersection->y() = begin_.y() + ua*(end_.y() - begin_.y());
          }

          return INTERESECTING;
      }

      return NOT_INTERESECTING;
    }
  };

  // --

  /**
   *  Generates internal borders to a series of polygons
   **/
  class PolygonBorderGenerator {
  public:
    class BorderedPolygon {
    private:
      octet::dynarray<Polygon2D> _borders;
      Polygon2D _area;

    public:
      octet::dynarray<Polygon2D>& getBorders() {
        return _borders;
      };

      const octet::dynarray<Polygon2D>& getBorders() const {
        return _borders;
      };

      void setArea(const Polygon2D& area) {
        _area = area;
      };

      const Polygon2D& getArea() const {
        return _area;
      };

      float getTotalArea() const {
        float totalArea = _area.getArea();

        for (size_t i = 0; i < _borders.size(); ++i) {
          totalArea += _borders[i].getArea();
        }

        return totalArea;
      };
    };

  private:
    const octet::dynarray<Polygon2D>* _polygons;
    octet::dynarray<BorderedPolygon> _borders;
    
    void computeInnerBorderLineEquations(const Polygon2D& polygon, float offset, octet::dynarray<LineSegment>& results) const {
      Polygon2D::Vertex centroid = polygon.getCentroid();

      for (size_t i = 0; i < polygon.getVertexCount(); ++i) {
        Polygon2D::Edge edge = polygon.getEdge(i);
        //DEBUG_PRINT("edge: [%.2f, %.2f], [%.2f, %.2f]\n", edge.first().x(), edge.first().y(), edge.second().x(), edge.second().y());
        
        // Evaluate and normalize the vector representing the edge from vertex i directed towards the next vertex
        octet::vec2 edgeVector = edge.second() - edge.first();
        //DEBUG_PRINT("edgeVector: [%.2f, %.2f]\n", edgeVector.x(), edgeVector.y());
        edgeVector = edgeVector.normalize();
        //DEBUG_PRINT("edgeVector: [%.2f, %.2f]\n", edgeVector.x(), edgeVector.y());
        
        // Rotate the edge vector by 90 degrees (anti-clockwise) and scale it by offset
        octet::vec2 offsetVector(edgeVector.y() * -offset, edgeVector.x() * offset);
        //DEBUG_PRINT("offsetVector: [%.2f, %.2f]\n", offsetVector.x(), offsetVector.y());
        
        // If the vector towards the centroid and the offset vector do not point to the same relative direction,
        // reflect the offsetVector in order for it to point towards the centroid as well
        if ((centroid - edge.first()).dot(offsetVector) < 0) {
          offsetVector = -offsetVector;
        }
        //DEBUG_PRINT("offsetVector: [%.2f, %.2f]\n", offsetVector.x(), offsetVector.y());

        // Record the equation of the offset line
        results.push_back( LineSegment(edge.first() + offsetVector, edge.second() + offsetVector) );
      }
    };

    // Reference: http://forum.gpwiki.org/viewtopic.php?p=74970&sid=7d38851004ecff8b20e8edd788492c90#p74970
    BorderedPolygon& generate(const Polygon2D& polygon, float borderWidth, BorderedPolygon& result) {
      result.setArea(polygon);
      
      if (borderWidth == 0.f) {
        return result;
      }

      octet::dynarray<LineSegment> innerBorderLineEquations;
      innerBorderLineEquations.reserve(polygon.getVertexCount());

      // Compute the equations of the line of all of the inner borders
      computeInnerBorderLineEquations(polygon, borderWidth, innerBorderLineEquations);
      
      // The area which is not within the border bounds
      Polygon2D area;
      Polygon2D::Vertex vertex;
      for (size_t i = 0; i < innerBorderLineEquations.size(); ++i) {
        const LineSegment& previous = (i == 0 ? innerBorderLineEquations.back() : innerBorderLineEquations[i - 1]);
        previous.Intersect(innerBorderLineEquations[i], vertex);

        area.addVertex(vertex);
      }

      result.setArea(area);
      for (size_t i = 0; i < area.getVertexCount(); ++i) {
        result.getBorders().push_back(
          Polygon2D().addVertex(polygon[i]).
                      addVertex(area[i]).
                      addVertex(area[area.nextVertex(i)]).
                      addVertex(polygon[polygon.nextVertex(i)])
        );
      }

      return result;
    };

  public:
    PolygonBorderGenerator() {
    };

    ~PolygonBorderGenerator() {
    };
    
    PolygonBorderGenerator& setPolygons(const octet::dynarray<Polygon2D>* polygons) {
      _polygons = polygons;
      return *this;
    };

    /**
     *  @return the associated BorderedPolygon for the specified Polygon2D index
     */
    const BorderedPolygon& getBorderedPolygon(unsigned int polygonIndex) const {
      return _borders[polygonIndex];
    };
    
    /**
     *  Generates inner borders for the specifed polygons
     **/
    PolygonBorderGenerator& generate(float width) {
      if (_polygons != NULL) {
        _borders.reset();
        _borders.reserve(_polygons->size());

        for (size_t i = 0; i < _polygons->size(); ++i) {
          _borders.push_back(generate(_polygons->operator[](i), ::abs(width / 2.f)));
        };
      }

      return *this;
    };
    
    /**
     *  Generates inner borders for the specifed polygon
     **/
    BorderedPolygon generate(const Polygon2D& polygon, float width) {
      return generate(polygon, ::abs(width), BorderedPolygon());
    };
  };

} // namespace octet