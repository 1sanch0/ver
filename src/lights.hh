#ifndef LIGHTS_H_
#define LIGHTS_H_

#include "geometry.hh"

class LightPoint {
  public:
    LightPoint(const Point &point, const Direction &pw): p{point}, power{pw} {} // TODO: cambiar y tal

  public:
    Point p;
    Direction power;
};


#endif // LIGHTS_H_
