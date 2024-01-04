#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "ver.hh"
#include "geometry.hh"
#include "spectrum.hh"
#include "interaction.hh"

enum HemisphereSampler {
  SOLID_ANGLE, COSINE
};

class BSDF {
  public:
    explicit BSDF(bool isDelta_) : isDelta{isDelta_} {}

    virtual Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const = 0;
    virtual Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const = 0;
    virtual Float p(HemisphereSampler sampler, const Direction &wi) const = 0;
    virtual Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const = 0;
  
  public:
    bool isDelta;
};

class IMaterial {
  public:
    virtual std::shared_ptr<BSDF> sampleFr(const SurfaceInteraction &si) const = 0;

    virtual Spectrum Le() const = 0;
};

#endif // MATERIAL_H_
