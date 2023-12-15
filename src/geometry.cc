#include "geometry.hh"

void makeCoordSystem(const Direction &b1, Direction &b2, Direction &b3) {
  Float eps = 1e-2;
  Float x = std::abs(b1.norm() - 1.0);
  Float y = b1.norm();
  assert(std::abs(b1.norm() - 1.0) < eps, "Coordinate System constructions requires a normalize vector!");

  if (std::abs(b1.x) > std::abs(b1.y))
    b2 = Direction(-b1.z, 0, b1.x) / std::sqrt(b1.x * b1.x + b1.z * b1.z);
  else
    b2 = Direction(0, b1.z, -b1.y) / std::sqrt(b1.y * b1.y + b1.z * b1.z);
      
  b3 = b1.cross(b2);
}