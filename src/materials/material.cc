#include "material.hh"

Direction reflect(const Direction &v, const Direction &n) {
  return v + n * 2 * v.dot(n);
}

Direction refract(const Direction &v, const Direction &n, Float n1, Float n2) {
// Source: https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf

  const Float eta = n1 / n2;
  const Float cosI = -n.dot(v);
  const Float sinT2 = eta * eta * (1.0 - cosI * cosI);

  if (sinT2 > 1.0)
    return v - n * 2 * cosI;
  
  const Float cosT = std::sqrt(1.0 - sinT2);

  // TODO: Review
  const Float r0rth = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
  const Float rPar = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
  const Float r = (r0rth * r0rth + rPar * rPar) / 2.0;

  if (uniform(0, 1) < r)
    return v - n * 2 * cosI;

  return v * eta + n * (eta * cosI - cosT);
}
