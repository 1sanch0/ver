#include "primitive.hh"
#include "materials/material.hh"

GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape_,
                                       const std::shared_ptr<IMaterial> &material_) 
  : shape{shape_}, material{material_} {}

Bounds GeometricPrimitive::bounds() const { return shape->bounds(); }

bool GeometricPrimitive::intersect(const Ray &ray, SurfaceInteraction &interact) const {
  Float tHit;
  if (!shape->intersect(ray, tHit, interact)) return false;

  interact.material = material;
  // TODO: change normals if normal map in material
  return true;
}
