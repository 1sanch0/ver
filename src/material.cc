#include "material.hh"

namespace Slides {
  DiffuseBRDF::DiffuseBRDF(const ::Spectrum &coefficient, Float prob)
    : k{coefficient}, invProb{(Float)1.0 / prob} {} 

  ::Spectrum DiffuseBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return k * M_1_PI;
  }

  ::Spectrum DiffuseBRDF::sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const {
  // (Page: 11) https://moodle.unizar.es/add/pluginfile.php/9116118/mod_label/intro/ig_practica_8.pdf?time=1698147710097

    const Float theta = (sampler == SOLID_ANGLE) ? std::acos(uniform(0, 1))
                                    /* COSINE */ : std::acos(std::sqrt(1.0 - uniform(0, 1)));
    const Float phi = 2.0 * M_PI * uniform(0, 1);

    Direction x, y, z = si.n;
    makeCoordSystem(z, x, y); 
    const Mat4 transform(x, y, z);

    wi = transform * Direction(std::sin(theta) * std::cos(phi),
                              std::sin(theta) * std::sin(phi),
                              std::cos(theta));

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
    : k{coefficient}, invProb{(Float)1.0 / prob} {} 

  ::Spectrum PerfectSpecularBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  ::Spectrum PerfectSpecularBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    const auto n = si.n;
    const auto v = -si.wo;

    wi = v - n * 2 * v.dot(n);

    return k * invProb; // / wi.dot(n) ; gets cancelled out      <--
  }

  Float PerfectSpecularBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
    return 1.0;
  }
      
  Float PerfectSpecularBRDF::cosThetaI(HemisphereSampler /*sampler*/, const Direction &/*wi*/, const Direction &/*n*/) const {
    return 1.0; // wi.dot(n) gets cancelled out by the brdf      <--
  }


  RefractionBRDF::RefractionBRDF(const ::Spectrum &coefficient, Float prob) 
    : k{coefficient}, invProb{(Float)1.0 / prob} {} 

  ::Spectrum RefractionBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  ::Spectrum RefractionBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    // https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
    // TODO pls forsen help me
    const auto n = si.n;
    const auto v = -si.wo;
    const bool enters = v.dot(n) < 0; // Move to surface interaciton 

    // wi = v - n * 2 * v.dot(n);

    const Float n1 = (enters) ? 1.0 : 1.5;
    const Float n2 = (enters) ? 1.5 : 1.0;
    const Float nn = (n1/n2);

    const Float cosThetaI = n.dot(v);

    const Float sin2ThetaT = nn * nn * (1.0 - cosThetaI * cosThetaI);

    if (sin2ThetaT > 1.0) { // TIR
      wi = v - n * 2 * v.dot(n);
      return k * invProb; // / wi.dot(n) ; gets cancelled out           <--
    }

    const Float cosThetaT = std::sqrt(1.0 - sin2ThetaT);

    wi = v * nn + n * (nn * cosThetaI - cosThetaT);

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

  std::shared_ptr<BSDF> Material::sampleFr() const {
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