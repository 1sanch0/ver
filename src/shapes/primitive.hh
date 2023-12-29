#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

#include "ver.hh"
#include "geometry.hh"
#include "shapes/shape.hh"
#include "interaction.hh"

class Primitive {
  public:
    virtual Bounds bounds() const = 0;
    virtual bool intersect(const Ray &ray, SurfaceInteraction &interact) const = 0;
    // virtual std::shared_ptr<Material> material() const = 0;
};

class GeometricPrimitive : public Primitive {
  public:
    GeometricPrimitive(const std::shared_ptr<Shape> &shape_,
                       const std::shared_ptr<IMaterial> &material_);

    Bounds bounds() const override;
    bool intersect(const Ray &ray, SurfaceInteraction &interact) const override;
    // std::shared_ptr<Material> material() const override;
  private:
    std::shared_ptr<Shape> shape;
    std::shared_ptr<IMaterial> material;
};

#endif // PRIMITIVE_H_
