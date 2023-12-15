#include "primitive.hh"

GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape_,
                                       const std::shared_ptr<Material> &material_) 
  : shape{shape_}, material{material_} {}

Bounds GeometricPrimitive::bounds() const { return shape->bounds(); }

bool GeometricPrimitive::intersect(const Ray &ray, SurfaceInteraction &interact) const {
  Float tHit;
  if (!shape->intersect(ray, tHit, interact)) return false;

  interact.material = material;
  return true;
}
