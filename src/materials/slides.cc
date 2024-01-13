#include "slides.hh"

namespace Slides {
  DiffuseBRDF::DiffuseBRDF(const ::Spectrum &coefficient, Float prob)
    : BSDF{false}, k{coefficient}, invProb{(Float)1.0 / prob} {} 

  ::Spectrum DiffuseBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return k;// M_1_PI gets cancelled out
  }

  ::Spectrum DiffuseBRDF::sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const {
  // (Page: 11) https://moodle.unizar.es/add/pluginfile.php/9116118/mod_label/intro/ig_practica_8.pdf?time=1698147710097

    const Direction n = (si.entering) ? si.n : -si.n; // TODO: !!!!

    wi = randomHemisphereDirection(n, sampler);

    return k * invProb; // * M_1_PI; gets cancelled out     <--
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


  PerfectSpecularBRDF::PerfectSpecularBRDF(const ::Spectrum &coefficient, Float prob) 
    : BSDF{true}, k{coefficient}, invProb{(Float)1.0 / prob} {} 

  ::Spectrum PerfectSpecularBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  ::Spectrum PerfectSpecularBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    const Direction n = (si.entering) ? si.n : -si.n;

    wi = reflect(-si.wo, n);

    return k * invProb; // / wi.dot(n) ; gets cancelled out      <--
  }

  Float PerfectSpecularBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
    return 1.0;
  }
      
  Float PerfectSpecularBRDF::cosThetaI(HemisphereSampler /*sampler*/, const Direction &/*wi*/, const Direction &/*n*/) const {
    return 1.0; // wi.dot(n) gets cancelled out by the brdf      <--
  }


  RefractionBRDF::RefractionBRDF(const ::Spectrum &coefficient, Float prob) 
    : BSDF{true}, k{coefficient}, invProb{(Float)1.0 / prob} {} 

  ::Spectrum RefractionBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  ::Spectrum RefractionBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    const Float n1 = si.entering ? 1.0 : 1.5; // TODO: change to variable
    const Float n2 = si.entering ? 1.5 : 1.0;

    const Direction n = (si.entering) ? si.n : -si.n;

    wi = refract(-si.wo, n, n1, n2);

    return k * invProb; // / wi.dot(n) ; gets cancelled out     <--
  }

  Float RefractionBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
    return 1.0;
  }

  Float RefractionBRDF::cosThetaI(HemisphereSampler /*sampler*/, const Direction &/*wi*/, const Direction &/*n*/) const {
    return 1.0; // wi.dot(n) gets cancelled out by the brdf     <--
  }


  Material::Material(const ::Spectrum &kd, const ::Spectrum &ks, const ::Spectrum &kt, const ::Spectrum &ke,
          Float eta_)
          : emission{ke}, eta{eta_},
            prob_d{kd.max()}, prob_s{ks.max()}, prob_t{kt.max()} { 
    auto k = kd + ks + kt;
    assert(k.max() <= 1, "BSDFs coefficients sum > 1");

    diffuse = std::make_shared<DiffuseBRDF>(kd, prob_d);
    specular = std::make_shared<PerfectSpecularBRDF>(ks, prob_s);
    refraction = std::make_shared<RefractionBRDF>(kt, prob_t);
  }

  std::shared_ptr<BSDF> Material::sampleFr(const SurfaceInteraction &/*si*/) const {
    const Float sample = uniform(0, 1);

    if (sample < prob_d) {
      return diffuse;
    } else if (sample < prob_d + prob_s) {
      return specular;
    } else if (sample < prob_d + prob_s + prob_t) {
      return refraction;
    } else {
      return nullptr;
    }
  }

  Spectrum Material::Le() const {
    return emission;
  }

}