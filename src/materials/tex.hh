#ifndef TEX_H_
#define TEX_H_

#include "material.hh"
#include "texture.hh"

namespace tex { // Texture-based materials
  class DiffuseBRDF : public BSDF {
    public:
      explicit DiffuseBRDF(const std::shared_ptr<Texture> &coefficient);

      Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const override;
      Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const override;
      Float p(HemisphereSampler sampler, const Direction &wi) const override;
      Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const override;

    private:
      std::shared_ptr<Texture> k;

    friend class Material;
  };

  class PerfectSpecularBRDF : public BSDF {
    public:
      explicit PerfectSpecularBRDF(const std::shared_ptr<Texture> &coefficient);

      Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const override;
      Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const override;
      Float p(HemisphereSampler sampler, const Direction &wi) const override;
      Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const override;

    private:
      std::shared_ptr<Texture> k;

    friend class Material;
  };

  class RefractionBRDF : public BSDF {
    public:
      explicit RefractionBRDF(const std::shared_ptr<Texture> &coefficient);

      Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const override;
      Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const override;
      Float p(HemisphereSampler sampler, const Direction &wi) const override;
      Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const override;

    private:
      std::shared_ptr<Texture> k;
    
    friend class Material;
  };
  
  // SampledBSDF is a BSDF that was sampled from a material using Russian Roulette
  class SampledBSDF : public BSDF {
    public:
      SampledBSDF(const std::shared_ptr<BSDF> &bsdf_, Float prob);

      Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const override;
      Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const override;
      Float p(HemisphereSampler sampler, const Direction &wi) const override;
      Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const override;

    private:
      std::shared_ptr<BSDF> bsdf;
      Float invProb;
  };

  class Material : public IMaterial {
    public:
      Material(const std::shared_ptr<Texture> &kd, const std::shared_ptr<Texture> &ks, const std::shared_ptr<Texture> &kt,
              const Spectrum &ke = Spectrum(0.0, 0.0, 0.0),
              Float eta_ = 1.0);

      std::shared_ptr<BSDF> sampleFr(const SurfaceInteraction &si) const override;

      Spectrum Le() const override;

    private:
      std::shared_ptr<DiffuseBRDF> diffuse;
      std::shared_ptr<PerfectSpecularBRDF> specular;
      std::shared_ptr<RefractionBRDF> refraction;
      Spectrum emission;

      Float eta;
  };
}

#endif // TEX_H_
