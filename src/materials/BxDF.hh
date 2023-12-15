#ifndef BXDF_H_
#define BXDF_H_

#include "geometry.hh"
#include "spectrum.hh"

enum BxDFType {
  REFLECTION = 1 << 0,
  TRANSMISSION = 1 << 1,
  DIFFUSE = 1 << 2,
  GLOSSY = 1 << 3, // TODO: glossy = diffuse + specular?
  SPECULAR = 1 << 4,
  ALL = REFLECTION | TRANSMISSION | DIFFUSE | GLOSSY | SPECULAR,
};

template <BxDFType type>
class BxDF {
  public:
    virtual Spectrum f(const Direction &wo, const Direction &wi) const = 0;
    virtual Spectrum sample_f(const Direction &wo, Direction &wi) const = 0;
};


#endif // BXDF_H_
