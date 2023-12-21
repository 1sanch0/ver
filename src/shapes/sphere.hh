#ifndef SPHERE_H_
#define SPHERE_H_

#include "shape.hh"

class Sphere : public Shape {
  public:
    Sphere(const Point &origin, Float radius);
    Bounds bounds() const override;
    bool intersect(const Ray &ray, Float &tHit,
                   SurfaceInteraction &interact) const override;
    Float area() const override;

  private:
    Point o;
    Float r;
};

#endif // SPHERE_H_
