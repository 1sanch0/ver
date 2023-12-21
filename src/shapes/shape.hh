#ifndef SHAPE_H_
#define SHAPE_H_

#include "ver.hh"
#include "geometry.hh"
#include "interaction.hh"

class Shape {
  public:
    virtual Bounds bounds() const = 0;
    virtual bool intersect(const Ray &ray, Float &tHit,
                           SurfaceInteraction &interact) const = 0;
    virtual bool intersect(const Ray &ray) const;
    virtual Float area() const = 0;
};



// TODO: disk, curve

#endif // SHAPE_H_
