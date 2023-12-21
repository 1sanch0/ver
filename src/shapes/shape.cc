#include "shape.hh"

bool Shape::intersect(const Ray &ray) const {
  Float tHit;
  SurfaceInteraction interact;
  return intersect(ray, tHit, interact);
}
