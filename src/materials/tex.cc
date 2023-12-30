#include "tex.hh"

namespace tex {
  DiffuseBRDF::DiffuseBRDF(const std::shared_ptr<Texture> &coefficient)
    : k{coefficient} {}
  
  Spectrum DiffuseBRDF::fr(const SurfaceInteraction &si, const Direction &/*wi*/) const {
    return k->value(si) * M_1_PI;
  }

  Spectrum DiffuseBRDF::sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const {
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
    : k{coefficient} {}
  
  Spectrum PerfectSpecularBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  Spectrum PerfectSpecularBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    const auto n = si.n;
    const auto v = -si.wo;

    wi = v - n * 2 * v.dot(n);

    return k->value(si); // / wi.dot(n) ; gets cancelled out      <--
  }

  Float PerfectSpecularBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
    return 1.0;
  }

  Float PerfectSpecularBRDF::cosThetaI(HemisphereSampler /*sampler*/, const Direction &/*wi*/, const Direction &/*n*/) const {
    return 1.0; // wi.dot(n) gets cancelled out by the brdf      <--
  }

  RefractionBRDF::RefractionBRDF(const std::shared_ptr<Texture> &coefficient) 
    : k{coefficient} {}
  
  Spectrum RefractionBRDF::fr(const SurfaceInteraction &/*si*/, const Direction &/*wi*/) const {
    return Spectrum(); // Delta function
  }

  Spectrum RefractionBRDF::sampleFr(HemisphereSampler /*sampler*/, const SurfaceInteraction &si, Direction &wi) const {
    // https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
    // TODO: probably wrong

    const auto n = si.n;
    const auto v = -si.wo;
    const bool enters = v.dot(n) < 0;

    const Float n1 = (enters) ? 1.0 : 2;
    const Float n2 = (enters) ? 2 : 1.0;
    const Float nn = n1 / n2;

    const Float cosI = v.dot(n);
    const Float sinT2 = nn * nn * (1.0 - cosI * cosI);

    if (sinT2 > 1.0) { // Total internal reflection
      wi = v - n * 2 * v.dot(n);
      return k->value(si); // / wi.dot(n) ; gets cancelled out      <--
    }

    // Fresnel effect (Schlick's approximation) // Move to function
    Float r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;
    Float cosX = (enters) ? -cosI : std::sqrt(1.0 - sinT2);
    if (n1 > n2)
      cosX = std::sqrt(1.0 - sinT2);
    const Float x = 1.0 - cosX;
    const Float r = r0 + (1.0 - r0) * x * x * x * x * x;

    if (uniform(0, 1) < r) { // Reflect
      wi = v - n * 2 * v.dot(n);
      return k->value(si); // / wi.dot(n) ; gets cancelled out      <--
    }

    const Float cosT = std::sqrt(1.0 - sinT2);
    wi = v * nn + n * (nn * cosI - cosT);

    return k->value(si); // / wi.dot(n) ; gets cancelled out      <--
  }

  Float RefractionBRDF::p(HemisphereSampler /*sampler*/, const Direction &/*wi*/) const {
    return 1.0;
  }

  Float RefractionBRDF::cosThetaI(HemisphereSampler /*sampler*/, const Direction &/*wi*/, const Direction &/*n*/) const {
    return 1.0; // wi.dot(n) gets cancelled out by the brdf     <--
  }

  SampledBSDF::SampledBSDF(const std::shared_ptr<BSDF> &bsdf_, Float prob) 
    : bsdf{bsdf_}, invProb{(Float)1.0 / prob} {}
  
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
      
      // TODO: check values < 1
      // else rescale them
  }

  std::shared_ptr<BSDF> Material::sampleFr(const SurfaceInteraction &si) const {
    const Float sample = uniform(0, 1);

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