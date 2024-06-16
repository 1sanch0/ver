#include "triangle.hh"

TriangleMesh::TriangleMesh(const Mat4 &transform, const simply::PLYFile &ply) {
  
  simply::PLYIndexer XIdx, YIdx, ZIdx;
  simply::PLYIndexer NxIdx, NyIdx, NzIdx;
  simply::PLYIndexer UIdx, VIdx;

  simply::PLYIndexer indicesIdx;

  bool hasPos = (ply.getPropertyIndexer("x", XIdx) &&
                 ply.getPropertyIndexer("y", YIdx) &&
                 ply.getPropertyIndexer("z", ZIdx));

  bool hasNormals = (ply.getPropertyIndexer("nx", NxIdx) &&
                     ply.getPropertyIndexer("ny", NyIdx) &&
                     ply.getPropertyIndexer("nz", NzIdx));

  bool hasUVs = (ply.getPropertyIndexer("u", UIdx) &&
                 ply.getPropertyIndexer("v", VIdx))
             || (ply.getPropertyIndexer("s", UIdx) &&
                 ply.getPropertyIndexer("t", VIdx));

  bool hasIndices = ply.getPropertyIndexer("vertex_indices", indicesIdx);
  //bool hasIndices = ply.getPropertyIndexer("vertex_index", indicesIdx);

  if (!hasPos) throw std::runtime_error("Could not found xyz properties");
  if (!hasIndices) throw std::runtime_error("Could not found vertex indices");

  assert(XIdx.amount == YIdx.amount && YIdx.amount == ZIdx.amount, "XYZ amounts dont match");
  for (size_t i = 0; i < XIdx.amount; i++) {
    Point pi;
    // TODO: correct dtype
    pi.x = ply.elements[XIdx.element_idx].take<float>(XIdx.start + XIdx.offset * i );
    pi.y = ply.elements[YIdx.element_idx].take<float>(YIdx.start + YIdx.offset * i );
    pi.z = ply.elements[ZIdx.element_idx].take<float>(ZIdx.start + ZIdx.offset * i );
    p.push_back(transform * pi);
  }

  if (hasNormals) {
    // TODO: assert size
    for (size_t i = 0; i < XIdx.amount; i++) {
      Direction ni;
      // TODO: correct dtype
      ni.x = ply.elements[NxIdx.element_idx].take<float>(NxIdx.start + NxIdx.offset * i );
      ni.y = ply.elements[NyIdx.element_idx].take<float>(NyIdx.start + NyIdx.offset * i );
      ni.z = ply.elements[NzIdx.element_idx].take<float>(NzIdx.start + NzIdx.offset * i );
      n.push_back((transform * ni).normalize());
    }
  }

  if (hasUVs) {
    std::cout << "hasUVs" << std::endl;
    for (size_t i = 0; i < XIdx.amount; i++) {
      Vec2 uvi;

      uvi.x = ply.elements[UIdx.element_idx].take<float>(UIdx.start + UIdx.offset * i );
      uvi.y = ply.elements[VIdx.element_idx].take<float>(VIdx.start + VIdx.offset * i );
      uv.push_back(uvi);

      std::cout << "uvi: " << uv.size() << ", " << uvi << std::endl;
    }
  }

  for (size_t i = 0; i < indicesIdx.amount; i++) {
    // TODO: correct datatypes
    int len = ply.elements[indicesIdx.element_idx].take<uint8_t>(indicesIdx.start);
    assert(len == 3, "Only triangles allowed");
    for (int j = 0; j < len; j++) {
      size_t idx = indicesIdx.start + (indicesIdx.ctype.size + indicesIdx.dtype.size * len) * i + indicesIdx.dtype.size * j + 1;
      indices.push_back(ply.elements[indicesIdx.element_idx].take<uint32_t>(idx));
    }
  }

  nTriangles = indices.size() / 3;
  nVertices = p.size();
}

TriangleMesh::TriangleMesh(const Mat4 &transform, std::vector<size_t> &vertexIndices,
                           std::vector<Point> &P,
                           std::vector<Direction> &N,
                           std::vector<Direction> &S,
                           std::vector<Vec2> &UV)
  : nTriangles{vertexIndices.size() / 3}, nVertices{P.size()}, indices{vertexIndices} {

  p.resize(nVertices);
  #pragma omp parallel for
  for (size_t i = 0; i < nVertices; i++)
    p[i] = transform * P[i];

  if (!N.empty()) {
    n.resize(nVertices);
    #pragma omp parallel for
    for (size_t i = 0; i < nVertices; i++)
      n[i] = transform * N[i];
  }

  if (!S.empty()) {
    s.resize(nVertices);
    #pragma omp parallel for
    for (size_t i = 0; i < nVertices; i++)
      s[i] = transform * S[i];
  }

  if (!UV.empty()) {
    uv.resize(nVertices);
    #pragma omp parallel for
    for (size_t i = 0; i < nVertices; i++)
      uv[i] = UV[i];
  }

}


Triangle::Triangle(const std::shared_ptr<TriangleMesh> &triangleMesh, size_t triangle)
  : mesh{triangleMesh}, v{&mesh->indices[3 * triangle]} { }

Bounds Triangle::bounds() const {
  // TODO: transformation??
  const Point &p0 = mesh->p[v[0]];
  const Point &p1 = mesh->p[v[1]];
  const Point &p2 = mesh->p[v[2]];
  // std::cout << "p0: " << p0 << std::endl;
  // std::cout << "p1: " << p1 << std::endl;
  // std::cout << "p2: " << p2 << std::endl;
  // std::cout << Bounds(p0).Union(p1).Union(p2) << std::endl;
  // std::cout << Bounds(p0).Union(p1).Union(p2).volume() << std::endl << std::endl;
  return Bounds(p0).Union(p1).Union(p2);
}

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
bool Triangle::intersect(const Ray &ray, Float &tHit,
                         SurfaceInteraction &interact) const {
  constexpr Float eps = std::numeric_limits<Float>::epsilon();

  const Point &p0 = mesh->p[this->v[0]];
  const Point &p1 = mesh->p[this->v[1]];
  const Point &p2 = mesh->p[this->v[2]];

  // 1. Check if ray intersects triangle plane
  const Direction e1 = p1 - p0;
  const Direction e2 = p2 - p0;
  const Direction ray_x_e2 = ray.d.cross(e2);
  const Float det = e1.dot(ray_x_e2);

  if (det > -eps && det < eps) return false; // ray parallel to triangle plane

  // 2. Check if ray intersects triangle
  // Barycentric coordinates to define a point: P = w * p0 + u * p1 + v * p2,
  // where w + u + v = 1, so w = 1 - u - v. Then:
  // P = (1 - u - v) * p0 + u * p1 + v * p2, or
  // P = p0 + u * (p1 - p0) + v * (p2 - p0), or
  // P = p0 + u * e1 + v * e2
  //
  // Then, we can solve for u and v:
  // ray.o + t * ray.d = p0 + u * e1 + v * e2, or
  // ray.o - p0 = -t * ray.d + u * e1 + v * e2
  // This is a system of linear equations (Ax = b),
  // where: A = [-ray.d, e1, e2], x = [t, u, v], b = ray.o - p0
  // and can be solved using Cramer's rule: https://en.wikipedia.org/wiki/Cramer%27s_rule

  const Float inv_det = 1.0 / det;
  const Direction b = ray.o - p0;

  const Float u = b.dot(ray_x_e2) * inv_det;
  if (u < 0.0 || u > 1.0) return false;

  const Direction ray_x_e1 = b.cross(e1);
  const Float v = ray.d.dot(ray_x_e1) * inv_det;
  if (v < 0.0 || u + v > 1.0) return false;

  const Float w = 1.0 - u - v;

  const Float t = e2.dot(ray_x_e1) * inv_det;

  if (t < eps) return false; // triangle behind ray

  // 3. Compute intersection information
  Vec2 uv[3];
  getUVs(uv);

  Vec2 uvHit = uv[0] * w + uv[1] * u + uv[2] * v;

  tHit = t;
  interact.p = ray(t);
  interact.t = t;
  interact.n = mesh->n[this->v[0]]; // TODO: improve 
  interact.wo = -ray.d;
  interact.entering = interact.n.dot(ray.d) < 0;

  interact.u = uvHit[0];
  interact.v = uvHit[1];

  return true;
}

Float Triangle::area() const {
  const Point &p0 = mesh->p[v[0]];
  const Point &p1 = mesh->p[v[1]];
  const Point &p2 = mesh->p[v[2]];

  return 0.5 * (p1 - p0).cross(p2 - p0).norm();
}

void Triangle::getUVs(Vec2 uv[3]) const {
  if (mesh->uv.empty()) {
    uv[0] = Vec2(0, 0);
    uv[1] = Vec2(1, 0);
    uv[2] = Vec2(1, 1);
  } else {
    uv[0] = mesh->uv[v[0]];
    uv[1] = mesh->uv[v[1]];
    uv[2] = mesh->uv[v[2]];
  }
}