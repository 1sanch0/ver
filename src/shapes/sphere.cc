#include "sphere.hh"

Sphere::Sphere(const Point &origin, Float radius) : o{origin}, r{radius} {}

Bounds Sphere::bounds() const { 
  const Direction min(-r, -r, -r);
  const Direction max(r, r, r);
  return Bounds(o+min, o+max);
}

bool Sphere::intersect(const Ray &ray, Float &tHit,
                       SurfaceInteraction &interact) const {
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
  interact.wo = (ray.o - interact.p).normalize();

  Float m = 2.0 * std::sqrt(interact.n.x*interact.n.x + interact.n.y*interact.n.y + (interact.n.z + 1.0)*(interact.n.z + 1.0));
  interact.u = interact.n.x / m + 0.5;
  interact.v =  1.0 - (interact.n.y / m + 0.5);
  
  // interact.u = 0.5 + std::atan2(interact.n.z, interact.n.x) / (2 * M_PI);
  // interact.v = 0.5 - std::asin(interact.n.y) / M_PI;

  assert(!std::isnan(interact.u), "u is nan");
  assert(!std::isnan(interact.v), "u is nan");

  // TODO:remove
  interact.sphere = true;
  
  return true;
}

Float Sphere::area() const {
  return 4.0 * M_PI * r * r;
}