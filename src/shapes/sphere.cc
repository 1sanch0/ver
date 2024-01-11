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
  interact.wo = -ray.d;
  interact.entering = interact.n.dot(ray.d) < 0;

  // const Direction v = interact.n;
  // // TODO: Creo que tengo los nombres cambiados jeje
  // Float theta = std::acos(clamp(v.z, -1, 1));
  // Float phi = std::atan2(v.y, v.x);
  // phi = (phi < 0) ? phi + 2 * M_PI : phi;

  // interact.u = theta * M_1_PI;
  // interact.v = phi * M_1_PI * 0.5;

  // assert(!std::isnan(interact.u), "u is nan");
  // assert(!std::isnan(interact.v), "v is nan");

  // // Partial derivatives w.r.t. theta and phi
  // interact.du = Direction(-v.y, v.x, 0).normalize();
  // interact.dv = Direction(v.x * v.z, v.y * v.z, -std::sqrt(v.x * v.x + v.y * v.y)).normalize();
  
  return true;
}

Float Sphere::area() const {
  return 4.0 * M_PI * r * r;
}