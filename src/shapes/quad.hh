#ifndef QUAD_H_
#define QUAD_H_

#include "triangle.hh"

std::shared_ptr<TriangleMesh> Quad(const Point &origin, const Direction &u, const Direction &v, const Direction &n);

#endif // QUAD_H_
