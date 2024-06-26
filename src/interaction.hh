#ifndef INTERACTION_H_
#define INTERACTION_H_

#include "ver.hh"
#include "geometry.hh"
// #include "material.hh"

// TODO https://www.pbr-book.org/3ed-2018/Geometry_and_Transformations/Interactions

class Interaction {
  // TODO: all the MediumInterface and rays things
  public:
    Interaction(const Point &point, const Direction &normal, const Direction w_o, Float t);

  public:
    Point p;
    Direction n;
    Direction wo;
    Float t;
};

class IMaterial; // Forward declaration to avoid circular dependency

struct SurfaceInteraction { // TODO: Temporal
  Point p;
  Direction n;
  /* TEST */
  Float u, v;
  Direction du, dv; 
  bool sphere = false;
  /* TEST */
  Direction wo;
  bool entering;
  Float t;
  std::shared_ptr<IMaterial> material;
};

#endif // INTERACTION_H_
