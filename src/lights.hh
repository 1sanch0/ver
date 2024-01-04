#ifndef LIGHTS_H_
#define LIGHTS_H_

#include "geometry.hh"

class PointLight {
  public:
    PointLight(const Point &point, const Direction &pw)
      : p{point}, power{pw} {}

  public:
    Point p;
    Direction power;
};

#endif // LIGHTS_H_
