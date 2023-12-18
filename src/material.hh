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
    virtual Spectrum fr(const SurfaceInteraction &si, const Direction &wi) const = 0;
    virtual Spectrum sampleFr(HemisphereSampler sampler, const SurfaceInteraction &si, Direction &wi) const = 0;
    virtual Float p(HemisphereSampler sampler, const Direction &wi) const = 0;
    virtual Float cosThetaI(HemisphereSampler sampler, const Direction &wi, const Direction &n) const = 0;
};

class IMaterial {
  public:
    virtual std::shared_ptr<BSDF> sampleFr() const = 0;

    virtual Spectrum Le() const = 0;
};

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

#if 0
class Material {
  public:
    Material(const Direction &K_d, const Direction &K_s, const Direction &K_t, const Direction &K_e,
             Float eta_ = 1.0)
            : Kd{K_d}, Ks{K_s}, Kt{K_t}, Ke{K_e}, eta{eta_} {
      auto s = Kd + Ks + Kt;
      assert(s.max() <= 1, "Albedo sums >1")
    }
    
    // TODO: THIS IS THE MAIN ERROR FOR THE WRONG TOP
    Spectrum fr_diffuse(const SurfaceInteraction &si, const Direction &wo, Direction &wi) {
      const auto n = si.n;

      // Direction b2, b3;
      // if (si.sphere) {
      //   b3 = n.cross(wo);
      //   b2 = b3.cross(n);
      // } else
      //   makeCoordSystem(n, b2, b3);
      // const Mat4 transform(b3, b2, n);

      Direction x, y, z = n;
        makeCoordSystem(z, x, y); 

      const Mat4 transform(x, y, z);

      #if 1
      const Float theta = std::acos(std::sqrt(1.0 - uniform(0, 1))); // TODO: bien? sqrt(1-unifrom)
      const Float phi = 2.0 * M_PI * uniform(0, 1);

      // TODO REVISAR LUZ DE AREA MUAL

      wi = transform * Direction(std::sin(theta) * std::cos(phi),
                                 std::sin(theta) * std::sin(phi),
                                 std::cos(theta));
      #else
      // Wrong but works
      const Float theta = std::acos(2 * uniform(0, 1) - 1);
      const Float phi = 2 * M_PI * uniform(0, 1);

      wi = Direction(std::sin(theta) * std::cos(phi),
                     std::sin(theta) * std::sin(phi),
                     std::cos(theta));
      #endif

      return Kd; //* M_1_PI;
    }

    Spectrum fr_specular(const SurfaceInteraction &si, const Direction &wo, Direction &wi) {
      const auto n = si.n;
      const auto v = -wo;

      wi = v - n * 2 * v.dot(n);

      return Ks;// / wi.dot(n);
    }
    
    Spectrum fr_refraction(const SurfaceInteraction &si, const Direction &wo, Direction &wi) {
      // https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
      const auto n = si.n;
      const auto v = -wo;
      const bool enters = wo.dot(n) > 0; // Move to surface interaciton 

      const Float n1 = (enters) ? 1.0 : 1.5;
      const Float n2 = (enters) ? 1.5 : 1.0;
      const Float nn = (n1/n2);

      wi = v - n * 2 * v.dot(n);

      // const Float cosThetaI = n.dot(v);

      // const Float sin2ThetaT = nn * nn * (1.0 - cosThetaI * cosThetaI);

      // Float r0 = (1-nn) / (1+nn);
      // r0 = r0*r0;
      // const Float R= r0 + (1-r0)*pow((1 - cosThetaI),5);

      // if (sin2ThetaT > 1.0 || R > uniform(0, 1)) { // TIR
      //   wi = v + n * 2 * cosThetaI;
      //   return Kt;
      // }

      // const Float cosThetaT = std::sqrt(1.0 - sin2ThetaT);

      // wi = v * nn + n * (nn * cosThetaI - cosThetaT);
      return Kt;

      // const Float n1 = (enters) ? 1.0 : 1.5;
      // const Float n2 = (enters) ? 1.5 : 1.0;
      // const Float nn = (n1/n2);

      // const Float cosThetaI = -i.dot(n);

      // const Float sin2ThetaT = nn * nn * (1 - cosThetaI * cosThetaI);

      // const bool TIR = sin2ThetaT > 1.0;

      // if (TIR) {
      //   wi = i - n * 2 * i.dot(n);
      //   return Kt;
      // }

      // const Float cosThetaT = std::sqrt(1.0 - sin2ThetaT);

      // wi = i * nn + n * (nn * cosThetaI - cosThetaT);

      // const Float R0 = std::pow((n1 - n2) / (n1 + n2), 2); 

      // Float Rschlick;
      // if (n1 <= n2) {
      //   Rschlick = R0 + (1.0 - R0) * std::pow(1.0 - cosThetaI, 5);
      // } else if (n1 > n2 && !TIR) {
      //   Rschlick = R0 + (1.0 - R0) * std::pow(1.0 - cosThetaT, 5);
      // } else {
      //   Rschlick = 1.0;
      // }

      // const Float T = 1.0 - Rschlick;

      // return Kt;
    }

    Spectrum fr_sample(const SurfaceInteraction &si, const Direction &wo, Direction &wi, bool &absorption, uint &e) {
      const Float sample = uniform(0, 1);
      absorption = false;

      const Float pd = Kd.max();
      const Float ps = Ks.max();
      const Float pt = Kt.max();
      if (sample < pd) {
        e = 0;
        return fr_diffuse(si, wo, wi) * (1.0 / pd);
      } else if (sample < pd + ps) {
        e = 1;
        return fr_specular(si, wo, wi) * (1.0 / ps);
      } else if (sample < pd + ps + pt) {
        e = 2;
        return fr_refraction(si, wo, wi) * (1.0 / pt);
      }
      e = 3;

      absorption = true;
      return Spectrum();
    }

    Spectrum Le() const {
      return Ke; // TODO: review
    }

  public:
    Direction Kd, Ks, Kt, Ke;
    Float eta;
};
#endif

#endif // MATERIAL_H_
