#ifndef SLIDES_H_
#define SLIDES_H_

#include "material.hh"

namespace Slides { // BSDFs & Materials as seen in class (Fall 2023) (Unizar, Graphic IT)
  class DiffuseBRDF : public BSDF {
    public:
      DiffuseBRDF(const ::Spectrum &coefficient, Float prob = 1.0);

      ::Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const override;
      ::Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const override;
      Float p(HemisphereSampler sampler, const Direction &wi) const override;
      Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const override;

    private:
      ::Spectrum k;
      Float invProb;
  };

  class PerfectSpecularBRDF : public BSDF {
    public:
      PerfectSpecularBRDF(const ::Spectrum &coefficient, Float prob = 1.0);

      ::Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const override;
      ::Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const override;
      Float p(HemisphereSampler sampler, const Direction &wi) const override;
      Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const override;

    private:
      ::Spectrum k;
      Float invProb;
  };

  class RefractionBRDF : public BSDF {
    public:
      RefractionBRDF(const ::Spectrum &coefficient, Float prob = 1.0);

      ::Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const override;
      ::Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const override;
      Float p(HemisphereSampler sampler, const Direction &wi) const override;
      Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const override;

    private:
      ::Spectrum k;
      Float invProb;
  };

  class Material : public IMaterial {
    public:
      Material(const ::Spectrum &kd, const ::Spectrum &ks, const ::Spectrum &kt, const ::Spectrum &ke,
              Float eta_ = 1.0);

      std::shared_ptr<BSDF> sampleFr() const override;

      Spectrum Le() const override;

    private:
      std::shared_ptr<BSDF> diffuse;
      std::shared_ptr<BSDF> specular;
      std::shared_ptr<BSDF> refraction;

      ::Spectrum emission;

      Float eta;

      Float prob_d;
      Float prob_s;
      Float prob_t;
  };
}

#endif // SLIDES_H_
