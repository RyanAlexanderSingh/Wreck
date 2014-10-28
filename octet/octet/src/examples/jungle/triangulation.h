#include <vector>
#include <list>

#include "../../poly2tri/poly2tri_unity.h"

namespace octet {

  /**
   *  An adaptor class for poly2tri which allows
   *  for Constrained Delaunay Triangulation.
   *
   *  Reference: https://code.google.com/p/poly2tri/
   */
  class Triangulation {
  public:
    class Triangle {
    public:
      struct Vertex {
        int index;
        const octet::vec2* point;

        explicit Vertex(int index = -1, const octet::vec2* point = NULL) :
          index(index),
          point(point) {
        };
      };

    private:
      Vertex _vertices[3];

      friend class Triangulation;

      Triangle(const octet::dynarray<octet::vec2>* points, int a, int b, int c) {
        _vertices[0] = Vertex(a, (points == NULL || a < 0 ? NULL : &points->operator[](a)));
        _vertices[1] = Vertex(b, (points == NULL || b < 0 ? NULL : &points->operator[](b)));
        _vertices[2] = Vertex(c, (points == NULL || c < 0 ? NULL : &points->operator[](c)));
      };

    public:
      Triangle() {
      };

      Triangle(const Triangle& rhs) {
      };

      const Vertex& operator[](size_t index) const {
        return _vertices[index];
      };

      const Vertex& a() const {
        return operator[](0);
      };
      
      const Vertex& b() const {
        return operator[](1);
      };
      
      const Vertex& c() const {
        return operator[](2);
      };
    };

  private:
    octet::dynarray<p2t::Point> _points;
    octet::dynarray<octet::dynarray<p2t::Point> > _holes;
    octet::dynarray<p2t::Point> _polyLine;
    
    octet::dynarray<octet::vec2> _vertices;
    octet::dynarray<Triangle> _triangles;
    
    std::vector<p2t::Point*> get(octet::dynarray<p2t::Point>& pointMap) const {
      return get(pointMap, 0, pointMap.size());
    };
    
    std::vector<p2t::Point*> get(octet::dynarray<p2t::Point>& pointMap, size_t start, size_t end) const {
      std::vector<p2t::Point*> polyLine;
      polyLine.reserve(std::max(end - start, 0U));

      for (size_t i = start; i < end; ++i) {
        polyLine.push_back(&(pointMap[i]));
      }

      return polyLine;
    };
    
    p2t::Point convert(const octet::vec2& point) const {
      return p2t::Point(point.x(), point.y());
    };

    int lookup(const octet::dynarray<p2t::Point>& points, const p2t::Point* point) const {
      for (size_t i = 0; point != NULL && i < points.size(); ++i) {
        if (&(points[i]) == point) {
          return i;
        }
      }

      return -1;
    };

    int getIndex(p2t::Point* point) const {
      int result = lookup(_polyLine, point);
      if (result < 0) {
        size_t offset = _polyLine.size();

        for (size_t i = 0; result < 0 && i < _holes.size(); ++i) {
          result = lookup(_holes[i], point);

          if (result > -1) {
            return result + offset;
          }

          offset += _holes[i].size();
        }

        result = lookup(_points, point);
        if (result > -1) {
          result += offset;
        }
      }

      return result;
    };

    int getIndex(octet::hash_map<p2t::Point*, unsigned int>& map, p2t::Point* point) const {
      if (map.size() > 0) {
        int index = map.get_index(point);
        if (index > -1) {
          return map.get_value(index);
        }
      }

      int result = getIndex(point);

      if (result > -1) {
        map[point] = result;
      }

      return result;
    };

    void set(const std::vector<p2t::Triangle*>& triangles) {
      _triangles.reset();
      _triangles.reserve(triangles.size());

      // NOTE There seems to be an issue when looking up pointers in empty maps...
      //octet::hash_map<p2t::Point*, unsigned int> map;

      std::vector<p2t::Triangle*>::const_iterator iterator = triangles.cbegin();
      for (; iterator != triangles.cend(); ++iterator) {
        p2t::Point* a = (*iterator)->GetPoint(0);
        p2t::Point* b = (*iterator)->PointCW(*a); //(*iterator)->GetPoint(1);
        p2t::Point* c = (*iterator)->PointCW(*b); //(*iterator)->GetPoint(2);

        _triangles.push_back( Triangle(&_vertices, getIndex(a), getIndex(b), getIndex(c)) );
      }
    };

    template <typename T>
    static typename octet::dynarray<T>::iterator getIterator(octet::dynarray<T>& list, unsigned int index) {
      typename octet::dynarray<T>::iterator iterator = list.begin();
      while (index > 0) {
        ++iterator;
        --index;
      }

      return iterator;
    };

  public:
    Triangulation() {
    };
      
    ~Triangulation() {
    };

    Triangulation& reset() {
      _points.reset();
      _holes.reset();
      _polyLine.reset();

      _vertices.reset();
      _triangles.reset();

      return *this;
    };
    
    size_t addPolygonPoint(const octet::vec2& point) {
      _polyLine.push_back(convert(point));
      _vertices.push_back(point);
      return _vertices.size() - 1;
    };
    
    std::pair<size_t, size_t> addHole(const octet::dynarray<vec2>& points) {
      _holes.push_back(octet::dynarray<p2t::Point>());
      octet::dynarray<p2t::Point>& p2tPoints = _holes.back();
      
      size_t base = _vertices.size();

      p2tPoints.reserve(points.size());
      for (size_t i = 0; i < points.size(); ++i) {
        p2tPoints.push_back(convert(points[i]));
        _vertices.push_back(points[i]);
      }

      return std::make_pair(base, _vertices.size() - 1);
    };

    size_t addPoint(const octet::vec2& point) {
      _points.push_back(convert(point));
      _vertices.push_back(point);
      return _vertices.size() - 1;
    };

    Triangulation& triangulate() {
      // Convert arguments to match poly2tri

      p2t::CDT cdt(get(_polyLine));

      for (size_t i = 0; i < _holes.size(); ++i) {
        cdt.AddHole(get(_holes[i]));
      }

      for (size_t i = 0; i < _points.size(); ++i) {
        cdt.AddPoint(&(_points[i]));
      }

      // Perform triangulation
      cdt.Triangulate();

      // Store results locally
      set(cdt.GetTriangles());

      return *this;
    };

    const octet::dynarray<Triangle>& getTriangles() const {
      return _triangles;
    };

    const octet::dynarray<octet::vec2>& getVertices() const {
      return _vertices;
    };
  };
} // namespace octet