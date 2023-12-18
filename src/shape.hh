#ifndef SHAPE_H_
#define SHAPE_H_

#include "ver.hh"
#include "geometry.hh"
#include "interaction.hh"
#include "io/simply.hh"

class Shape {
  public:
    virtual Bounds bounds() const = 0;
    virtual bool intersect(const Ray &ray, Float &tHit,
                           SurfaceInteraction &interact,
                           bool testAlphaTexture = true) const = 0;
    virtual bool intersect(const Ray &ray, bool testAlphaTexture = true) const;
    virtual Float area() const = 0;
    // TODO: interaction stuff
};

class Sphere : public Shape {
  public:
    Sphere(const Point &origin, Float radius);
    Bounds bounds() const override;
    bool intersect(const Ray &ray, Float &tHit,
                   SurfaceInteraction &interact,
                   bool testAlphaTexture = true) const override;
    Float area() const override;

  private:
    Point o;
    Float r;
};

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
                   SurfaceInteraction &interact,
                   bool testAlphaTexture = true) const override;
    Float area() const override;

  private:
    void getUVs(Vec2 uv[2]) const;

    std::shared_ptr<TriangleMesh> mesh;
    const size_t *v;

};

std::shared_ptr<TriangleMesh> Quad(const Point &origin, const Direction &u, const Direction &v, const Direction &n);

// TODO: disk, curve

#endif // SHAPE_H_
