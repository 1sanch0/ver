#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "shape.hh"
#include "utils/simply.hh"

using namespace utils;

struct TriangleMesh {
  TriangleMesh(const Mat4 &transform, const simply::PLYFile &ply);
  TriangleMesh(const Mat4 &transform, std::vector<size_t> &vertexIndices,
               std::vector<Point> &P,
               std::vector<Direction> &N,
               std::vector<Direction> &S,
               std::vector<Vec2> &UV);

  TriangleMesh(const TriangleMesh &) = delete;
  TriangleMesh &operator=(const TriangleMesh &) = delete;
  TriangleMesh(TriangleMesh &&) = delete;
  TriangleMesh &operator=(TriangleMesh &&) = delete;

  /*const*/ size_t nTriangles, nVertices;
  std::vector<size_t> indices;
  std::vector<Point> p;
  std::vector<Direction> n;
  std::vector<Direction> s;
  std::vector<Vec2> uv;
};


class Triangle : public Shape {
  public:
    Triangle(const std::shared_ptr<TriangleMesh> &mesh, size_t triangle);

    Bounds bounds() const override;
    bool intersect(const Ray &ray, Float &tHit,
                   SurfaceInteraction &interact) const override;
    Float area() const override;

  private:
    void getUVs(Vec2 uv[2]) const;

    std::shared_ptr<TriangleMesh> mesh;
    const size_t *v;
};

#endif // TRIANGLE_H_
