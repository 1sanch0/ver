#include "tex.hh"

namespace tex {
  DiffuseBRDF::DiffuseBRDF(const std::shared_ptr<Texture> &coefficient)
    : BSDF{false}, k{coefficient} {}
  
  Spectrum DiffuseBRDF::fr(const SurfaceInteraction &si, const Direction &/*wi*/) const {
    return k->value(si); // M_1_PI gets cancelled out
  }

  Spectrum DiffuseBRDF::sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const {
  // (Page: 11) https://moodle.unizar.es/add/pluginfile.php/9116118/mod_label/intro/ig_practica_8.pdf?time=1698147710097

    const Direction n = (si.entering) ? si.n : -si.n; // TODO: !!!!

    wi = randomHemisphereDirection(n, sampler);

    return k->value(si); // * M_1_PI; gets cancelled out     <--
  }

  Float DiffuseBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
  // (Page: 12) https://moodle.unizar.es/add/pluginfile.php/9116118/mod_label/intro/ig_practica_8.pdf?time=1698147710097
    return 1.0; // Simplification, check slides!
  }

  Float DiffuseBRDF::cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const {
  // (Page: 12) https://moodle.unizar.es/add/pluginfile.php/9116118/mod_label/intro/ig_practica_8.pdf?time=1698147710097
    return (sampler == SOLID_ANGLE) ? 2.0 * wi.dot(n) /* TODO: abs*/
                                    : 1.0;
    // In both cases PI gets cancelled out by the BRDF      <--
  }

  PerfectSpecularBRDF::PerfectSpecularBRDF(const std::shared_ptr<Texture> &coefficient) 
    : BSDF{true}, k{coefficient} {}
  
  Spectrum PerfectSpecularBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  Spectrum PerfectSpecularBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    const Direction n = (si.entering) ? si.n : -si.n;

    wi = reflect(-si.wo, n);

    return k->value(si); // / wi.dot(n) ; gets cancelled out      <--
  }

  Float PerfectSpecularBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
    return 1.0;
  }

  Float PerfectSpecularBRDF::cosThetaI(HemisphereSampler /*sampler*/, const Direction &/*wi*/, const Direction &/*n*/) const {
    return 1.0; // wi.dot(n) gets cancelled out by the brdf      <--
  }

  RefractionBRDF::RefractionBRDF(const std::shared_ptr<Texture> &coefficient) 
    : BSDF{true}, k{coefficient} {}
  
  Spectrum RefractionBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  Spectrum RefractionBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    const Float n1 = si.entering ? 1.0 : 1.5; // TODO: change to variable
    const Float n2 = si.entering ? 1.5 : 1.0;

    const Direction n = (si.entering) ? si.n : -si.n;

    wi = refract(-si.wo, n, n1, n2);

    return k->value(si); // / wi.dot(n) ; gets cancelled out      <--
  }

  Float RefractionBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
    return 1.0;
  }

  Float RefractionBRDF::cosThetaI(HemisphereSampler /*sampler*/, const Direction &/*wi*/, const Direction &/*n*/) const {
    return 1.0; // wi.dot(n) gets cancelled out by the brdf     <--
  }

  SampledBSDF::SampledBSDF(const std::shared_ptr<BSDF> &bsdf_, Float prob) 
    : BSDF{bsdf_->isDelta}, bsdf{bsdf_}, invProb{(Float)1.0 / prob} {}
  
  Spectrum SampledBSDF::fr(const SurfaceInteraction &si, const Direction &wi) const {
    return bsdf->fr(si, wi);
  }

  Spectrum SampledBSDF::sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const {
    return bsdf->sampleFr(sampler, si, wi) * invProb;
  }

  Float SampledBSDF::p(HemisphereSampler sampler, const Direction &wi) const {
    return bsdf->p(sampler, wi);
  }

  Float SampledBSDF::cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const {
    return bsdf->cosThetaI(sampler, wi, n);
  }

  Material::Material(const std::shared_ptr<Texture> &kd, const std::shared_ptr<Texture> &ks, const std::shared_ptr<Texture> &kt,
              const Spectrum &ke, Float eta_)
    : diffuse{std::make_shared<DiffuseBRDF>(kd)},
      specular{std::make_shared<PerfectSpecularBRDF>(ks)},
      refraction{std::make_shared<RefractionBRDF>(kt)},
      emission{ke}, eta{eta_} {
      
    assert(kd && ks && kt, "Error: Material: nullptr texture")

    // Check that for all pixels, the sum of the coefficients is <= 1
    // assert(kd->max() + ks->max() + kt->max() <= 1, "BSDFs coefficients sum > 1");

    // TODO: review 
  }

  std::shared_ptr<BSDF> Material::sampleFr(const SurfaceInteraction &si) const {
    const Float sample = uniform(0, 1);

    assert(
      (diffuse->k->value(si) + specular->k->value(si) + refraction->k->value(si)).max() <= 1,
      "BSDFs coefficients sum > 1"
    )

    const Float probD = diffuse->k->value(si).max();
    const Float probS = specular->k->value(si).max();
    const Float probT = refraction->k->value(si).max();

    if (sample < probD) {
      return std::make_shared<SampledBSDF>(diffuse, probD); // Possible improvement: Arena allocation
    } else if (sample < probD + probS) {
      return std::make_shared<SampledBSDF>(specular, probS);
    } else if (sample < probD + probS + probT) {
      return std::make_shared<SampledBSDF>(refraction, probT);
    } else {
      return nullptr;
    }
  }

  Spectrum Material::Le() const {
    return emission;
  }
}