#include "quad.hh"

std::shared_ptr<TriangleMesh> Quad(const Point &origin, const Direction &u, const Direction &v, const Direction &n) {
  std::vector<Point> p = {origin + u + v, // Top right
                          origin - u + v, // Bottom right
                          origin - u - v, // Bottom left
                          origin + u - v};// Top left
  
  std::vector<size_t> indices = {0, 1, 3, 1, 2, 3};
  std::vector<Direction> N = {n, n, n, n, n, n};
  std::vector<Direction> s;
  std::vector<Vec2> uv = {Vec2(1, 1), Vec2(0, 1), Vec2(0, 0), Vec2(1, 0)};

  return std::make_shared<TriangleMesh>(Mat4::identity(), indices, p, N, s, uv);
}