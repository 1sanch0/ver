#ifndef INTERACTION_H_
#define INTERACTION_H_

#include "ver.hh"
#include "geometry.hh"
// #include "material.hh"

// TODO https://www.pbr-book.org/3ed-2018/Geometry_and_Transformations/Interactions

class Interaction {
  // TODO: all the MediumInterface and rays things
  public:
    Interaction(const Point &point, const Direction &normal, const Direction w_o, Float u, Float v);

  public:
    Point p;
    Direction wo;
    Direction n;
};

namespace Slides {
  class Material;
}
struct SurfaceInteraction { // TODO: Temporal
  Point p;
  Direction n;
  /* TEST */
  Direction du, dv; 
  bool sphere = false;
  /* TEST */
  Direction wo;
  Float t;
  std::shared_ptr<Slides::Material> material;
};

#endif // INTERACTION_H_
