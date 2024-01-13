#include "material.hh"

Direction randomHemisphereDirection(const Direction &n, HemisphereSampler sampler) {
  const Float theta = (sampler == SOLID_ANGLE) ? std::acos(uniform(0, 1))
                                  /* COSINE */ : std::acos(std::sqrt(1.0 - uniform(0, 1)));
  const Float phi = 2.0 * M_PI * uniform(0, 1);

  Direction x, y, z = n;
  makeCoordSystem(z, x, y); 
  const Mat4 transform(x, y, z);

  return transform * Direction(std::sin(theta) * std::cos(phi),
                               std::sin(theta) * std::sin(phi),
                               std::cos(theta));
}

Direction reflect(const Direction &v, const Direction &n) {
  const Float cosI = -n.dot(v);
  return v + n * 2 * cosI;
}

Direction refract(const Direction &v, const Direction &n, Float n1, Float n2) {
// Source: https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf

  const Float eta = n1 / n2;
  const Float cosI = -n.dot(v);
  const Float sinT2 = eta * eta * (1.0 - cosI * cosI);

  if (sinT2 > 1.0)
    return v + n * 2 * cosI;
  
  const Float cosT = std::sqrt(1.0 - sinT2);

  // Fresnel does not work :(( why? TODO
  #if 1
  // Reflectance
  const Float r0rth = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
  const Float rPar = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
  const Float r = (r0rth * r0rth + rPar * rPar) / 2.0;
  #else
  // Schlick's approximation
  Float r0 = (n1 - n2) / (n1 + n2);
  r0 *= r0;
  const Float cosX = (n1 < n2) ? cosI : cosT;
  const Float x = 1.0 - cosX;
  const Float r = r0 + (1.0 - r0) * x * x * x * x * x; 
  #endif

  if (uniform(0, 1) < r)
    return v + n * 2 * cosI;

  return v * eta + n * (eta * cosI - cosT);
}
