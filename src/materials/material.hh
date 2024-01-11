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


// Helper functions:

// Returns the reflected direction
Direction reflect(const Direction &v, const Direction &n);

// Returns the refracted direction
Direction refract(const Direction &v, const Direction &n, Float n1, Float n2);

#endif // MATERIAL_H_
