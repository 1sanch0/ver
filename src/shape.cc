#include "shape.hh"

bool Shape::intersect(const Ray &ray, bool testAlphaTexture) const {
  Float tHit;
  SurfaceInteraction interact;
  return intersect(ray, tHit, interact, testAlphaTexture);
}

Sphere::Sphere(const Point &origin, Float radius) : o{origin}, r{radius} {}

Bounds Sphere::bounds() const { 
  const Direction min(-r, -r, -r);
  const Direction max(r, r, r);
  return Bounds(o+min, o+max);
}

bool Sphere::intersect(const Ray &ray, Float &tHit,
                       SurfaceInteraction &interact,
                       bool testAlphaTexture) const {
  // https://link.springer.com/content/pdf/10.1007/978-1-4842-4427-2_7.pdf#0004286892.INDD%3AAnchor%2019%3A19
  Point G = o;
  Direction f = ray.o - G;

  Float b = (-f).dot(ray.d);  // if b < 0 G is behind the ray
  Float c = f.dot(f) - r * r; // if c > 0, ray.o is outside the sphere;

  Direction l = f + ray.d * b;
  Float discrim = r*r - l.dot(l);

  if (discrim < 0) return false;

  Float q = b + sign(b) * std::sqrt(discrim);

  Float t0 = c / q;
  Float t1 = q;

  if (t1 < t0) std::swap(t0, t1); // t0 will be less than or equal to t1
  if (t1 <= 0) return false;

  tHit = (t0 <= 0) ? t1 : t0;

  interact.n = (ray(tHit) - G) / r;

  // TODO:remove
  interact.p = ray(tHit);
  interact.t = tHit;
  interact.wo = (ray.o - ray(tHit)).normalize();

  // TODO: review (esta mal pero buen (hace fatla cambiar de base))
  // Float azimuth = std::atan2(interact.p.y / r, interact.p.x / r);

  // Float tmp = interact.p.z / r;
  // Float inclination = std::acos((tmp < 1) ? tmp : 1);

  // Direction longitude = Direction(
  //     std::cos(azimuth) * std::sin(inclination),
  //     std::sin(azimuth) * std::sin(inclination),
  //     std::cos(inclination)
  //   ).normalize();// TODO (this.planet.basis); // Cambio de base

  // Direction latitude = Direction(
  //     -std::sin(azimuth) * std::sin(inclination),
  //     std::cos(azimuth) * std::sin(inclination),
  //     0
  //   ).normalize(); //TODO (this.planet.basis); // Cambio de base
  
  // interact.du = longitude;
  // interact.dv = latitude;
  interact.sphere = true;
  
  return true;
}

Float Sphere::area() const {
  return 4.0 * M_PI * r * r;
}

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
                 ply.getPropertyIndexer("v", VIdx));

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

// Shout out: https://github.com/mmp/pbrt-v3/blob/13d871faae88233b327d04cda24022b8bb0093ee/src/shapes/triangle.cpp#L188
bool Triangle::intersect(const Ray &ray, Float &tHit,
                         SurfaceInteraction &interact,
                         bool testAlphaTexture) const {
  const Point &p0 = mesh->p[v[0]];
  const Point &p1 = mesh->p[v[1]];
  const Point &p2 = mesh->p[v[2]];

  // Translate vectices in world space to a new coordinate system where the orign is camera origin
  Point p0t = Point(p0 - ray.o);
  Point p1t = Point(p1 - ray.o);
  Point p2t = Point(p2 - ray.o);

  // Permute components of triangle vertices and ray direction
  int kz = ray.d.abs().argmax();
  int kx = kz + 1;
  if (kx == 3) kx = 0;
  int ky = kx + 1;
  if (ky == 3) ky = 0;
  Direction d = ray.d.permute(kx, ky, kz);
  p0t = p0t.permute(kx, ky, kz);
  p1t = p1t.permute(kx, ky, kz);
  p2t = p2t.permute(kx, ky, kz);

  // Apply shear transformation to translated vertex positions
  Float Sx = -d.x / d.z;
  Float Sy = -d.y / d.z;
  Float Sz = 1.f / d.z;
  p0t.x += Sx * p0t.z;
  p0t.y += Sy * p0t.z;
  p1t.x += Sx * p1t.z;
  p1t.y += Sy * p1t.z;
  p2t.x += Sx * p2t.z;
  p2t.y += Sy * p2t.z;

  // Compute edge function coefficients e0, e1, and e2
  Float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
  Float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
  Float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

  // Fall back to double precision test at triangle edges
  if (sizeof(Float) == sizeof(float) &&
    (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) {
    double p2txp1ty = (double)p2t.x * (double)p1t.y;
    double p2typ1tx = (double)p2t.y * (double)p1t.x;
    e0 = (float)(p2typ1tx - p2txp1ty);
    double p0txp2ty = (double)p0t.x * (double)p2t.y;
    double p0typ2tx = (double)p0t.y * (double)p2t.x;
    e1 = (float)(p0typ2tx - p0txp2ty);
    double p1txp0ty = (double)p1t.x * (double)p0t.y;
    double p1typ0tx = (double)p1t.y * (double)p0t.x;
    e2 = (float)(p1typ0tx - p1txp0ty);
  }

  // Perform triangle edge and determinant tests
  if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
    return false;
  Float det = e0 + e1 + e2;
  if (det == 0)
    return false;

  // Compute scaled hit distance to triangle and test against ray $t$ range
  p0t.z *= Sz;
  p1t.z *= Sz;
  p2t.z *= Sz;
  Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
  if (det < 0 && (tScaled >= 0 /*|| tScaled < ray.tMax * det */))
    return false;
  else if (det > 0 && (tScaled <= 0 /*|| tScaled > ray.tMax * det */))
    return false;

  // Compute barycentric coordinates and $t$ value for triangle intersection
  Float invDet = 1.0 / det;
  Float b0 = e0 * invDet;
  Float b1 = e1 * invDet;
  Float b2 = e2 * invDet;
  Float t = tScaled * invDet;

  // Ensure that computed triangle $t$ is conservatively greater than zero

  // Compute $\delta_z$ term for triangle $t$ error bounds
  Float maxZt = Direction(p0t.z, p1t.z, p2t.z).abs().argmax();
  Float deltaZ = gamma(3) * maxZt;

  // Compute $\delta_x$ and $\delta_y$ terms for triangle $t$ error bounds
  Float maxXt = Direction(p0t.x, p1t.x, p2t.x).abs().argmax();
  Float maxYt = Direction(p0t.y, p1t.y, p2t.y).abs().argmax();
  Float deltaX = gamma(5) * (maxXt + maxZt);
  Float deltaY = gamma(5) * (maxYt + maxZt);

  // Compute $\delta_e$ term for triangle $t$ error bounds
  Float deltaE = 2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

  // Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
  Float maxE = Direction(e0, e1, e2).abs().argmax();
  Float deltaT = 3 *
                  (gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
                  std::abs(invDet);
  if (t <= deltaT) return false;
  
  // INTERSECTION END

  // Compute triangle partial derivatives
  Direction dpdu, dpdv;
  Vec2 uv[3];
  getUVs(uv);

  // Compute deltas for triangle partial derivatives
  Vec2 duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
  Direction dp02 = p0 - p2, dp12 = p1 - p2;
  Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
  bool degenerateUV = std::abs(determinant) < 1e-8;
  if (!degenerateUV) {
      Float invdet = 1 / determinant;
      dpdu = (dp02 * duv12[1] - dp12 * duv02[1]) * invdet;
      dpdv = (dp02 * (-duv12[0]) + dp12 * duv02[0]) * invdet;
  }
  if (degenerateUV || (dpdu.cross(dpdv)).sqrNorm() == 0) {
      // Handle zero determinant for triangle partial derivative matrix
      Direction ng = (p2 - p0).cross(p1 - p0);
      if (ng.sqrNorm() == 0)
          // The triangle is actually degenerate; the intersection is
          // bogus.
          return false;

      // makeCoordSystem(ng.normalize(), dpdu, dpdv);
  }

  // Compute error bounds for triangle intersection
  Float xAbsSum =
      (std::abs(b0 * p0.x) + std::abs(b1 * p1.x) + std::abs(b2 * p2.x));
  Float yAbsSum =
      (std::abs(b0 * p0.y) + std::abs(b1 * p1.y) + std::abs(b2 * p2.y));
  Float zAbsSum =
      (std::abs(b0 * p0.z) + std::abs(b1 * p1.z) + std::abs(b2 * p2.z));
  Direction pError = Direction(xAbsSum, yAbsSum, zAbsSum) * gamma(7);

  // Interpolate $(u,v)$ parametric coordinates and hit point
  Point pHit = Point(Direction(p0) * b0 + Direction(p1) * b1 + Direction(p2) * b2);
  Vec2 uvHit = uv[0] * b0 + uv[1] * b1 + uv[2] * b2;

  // TODO: alpha texture

  // TODO: do better
  tHit = t;
  interact.p = pHit;
  interact.t = t;
  // interact.n = dp02.cross(dp12).normalize();
  interact.n = mesh->n[v[0]]; // TODO: will cause errors iun the future, interpolation good idea nice
  interact.wo = -ray.d;



  // // Compute the triangle's normal
  // auto p0p1 = p1 - p0;
  // auto p0p2 = p2 - p0;
  // auto n = p0p1.cross(p0p2);

  // // Check if the ray is parallel or nearly parallel to the triangle
  // Float nDotRayDir = n.dot(ray.d);
  // if (std::abs(nDotRayDir) < 1e-8) {
  //   return false;
  // }

  // // Compute the intersection parameter 't'
  // Float t = (p0 - ray.o).dot(n) / nDotRayDir;

  // // Check if the intersection point is behind the ray's origin or outside the triangle
  // if (t <= 0 /*|| t > ray.tMax*/) {
  //   return false;
  // }

  // // Compute the barycentric coordinates
  // auto pHit = ray(t);
  // auto p0p = pHit - p0;
  // Float u = p0p.cross(p0p2).dot(n) / n.dot(n);
  // Float v = p0p1.cross(p0p).dot(n) / n.dot(n);

  // // Check if the point is inside the triangle
  // if (u < 0.0 || v < 0.0 || u + v > 1.0) {
  //   return false;
  // }

  // Update the intersection information
  // tHit = t;
  // interact.p = pHit;
  // interact.t = t;
  // interact.n = n.normalize();
  // interact.wo = (ray.o - pHit).normalize();

  // You may want to compute other information like texture coordinates here

  return true;
}

Float Triangle::area() const {
  const Point &p0 = mesh->p[v[0]];
  const Point &p1 = mesh->p[v[1]];
  const Point &p2 = mesh->p[v[2]];

  return 0.5 * (p1 - p0).cross(p2 - p0).norm();
}

void Triangle::getUVs(Vec2 uv[2]) const {
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

std::shared_ptr<TriangleMesh> Quad(const Point &origin, const Direction &u, const Direction &v, const Direction &n) {
  std::vector<Point> p = {origin + u + v, // Top right
                          origin - u + v, // Bottom right
                          origin - u - v, // Bottom left
                          origin + u - v};// Top left
  
  std::vector<size_t> indices = {0, 1, 3, 1, 2, 3};
  std::vector<Direction> N = {n, n, n, n, n, n};
  std::vector<Direction> s;
  std::vector<Vec2> uv;

  return std::make_shared<TriangleMesh>(Mat4::identity(), indices, p, N, s, uv);
}
