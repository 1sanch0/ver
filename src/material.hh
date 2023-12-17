#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "ver.hh"
#include "geometry.hh"
#include "spectrum.hh"
#include "interaction.hh"

// class Sampler {
//   public:
//     virtual 
// }

// class BSDF {
//   public:
//     virtual Spectrum f(const SurfaceInteraction &si, const Direction &wo, const Direction &wi) const = 0;
//     virtual Spectrum sample_f(const SurfaceInteraction &si, const Direction &wo, Direction &wi, ) const = 0;
// };

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

      Direction b2, b3;
      if (si.sphere) {
        b2 = wo;
        b3 = n.cross(wo);
      } else
        makeCoordSystem(n, b2, b3);
      const Mat4 transform(b3, b2, n);

      // Direction x, y, z = n;
      // if (si.sphere) {
      //   y = wo;
      //   x = z.cross(y);
      // } else
      //   makeCoordSystem(z, x, y); 

      // const Mat4 transform(x, y, z);

      // #pragma omp critical
      // {
      //   std::cout << "---------" << std::endl;
      //   std::cout << wo.dot(n) << std::endl;
      //   std::cout << "n: " << n << std::endl;
      //   std::cout << "si.n: " << si.n << std::endl;
      //   std::cout << "T: " << std::endl;
      //   std::cout << transform << std::endl;
      // }

      const Float theta = std::acos(std::sqrt(1 - uniform(0, 1))); // TODO: bien? sqrt(1-unifrom)
      const Float phi = 2.0 * M_PI * uniform(0, 1);

      // TODO REVISAR LUZ DE AREA MUAL

      wi = transform * Direction(std::sin(theta) * std::cos(phi),
                                 std::sin(theta) * std::sin(phi),
                                 std::cos(theta));
      
      // #pragma omp critical
      // {
      //   std::cout << "theta: " << theta << " phi: " << phi << std::endl;
      //   std::cout << "wi': " << wi << std::endl;
      //   std::cout << "wi: ";
      //   std::cout << Direction(std::sin(theta) * std::cos(phi),
      //                          std::sin(theta) * std::sin(phi),
      //                          std::cos(theta)) << std::endl;
      //   std::cout << "+++++++++" << std::endl;
      // }
      
      // auto x = n;
      
      // if (x.x < 0)
      //   x.x = std::abs(x.x);
      // if (x.y < 0)
      //   x.y = std::abs(x.y);
      // if (x.z < 0)
      //   x.z = std::abs(x.z);
      // return x;
      return Kd; //* M_1_PI;
    }

    Spectrum fr_specular(const SurfaceInteraction &si, const Direction &wo, Direction &wi) {
      const auto n = si.n;
      const auto v = -wo;

      wi = v - n * 2 * (v.dot(n));

      return Ks;// / wi.dot(n);
    }
    
    Spectrum fr_refraction(const SurfaceInteraction &si, const Direction &wo, Direction &wi) {
      // https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
      const auto n = si.n;
      const auto i = -wo;
      const bool enters = wo.dot(n) > 0;

      const Float n1 = (enters) ? 1.0 : 1.5;
      const Float n2 = (enters) ? 1.5 : 1.0;
      const Float nn = (n1/n2);

      const Float cosThetaI = wo.dot(n);

      const Float sin2ThetaT = nn * nn * (1 - cosThetaI * cosThetaI);

      const bool TIR = sin2ThetaT > 1.0;

      if (TIR) {
        wi = i - n * 2 * i.dot(n);
        return Kt;
      }

      const Float cosThetaT = std::sqrt(1.0 - sin2ThetaT);

      wi = i * nn + n * (nn * cosThetaI - cosThetaT);

      const Float R0 = (n1 - n2) / (n1 + n2) * (n1 - n2) / (n1 + n2);

      Float Rschlick;
      if (n1 <= n2) {
        const Float a = (1 - cosThetaI);
        Rschlick = R0 + (1 - R0) * a * a * a * a * a;
      } else if (n1 > n2 && !TIR) {
        const Float a = (1 - cosThetaT);
        Rschlick = R0 + (1 - R0) * a * a * a * a * a;
      } else {
        Rschlick = 1.0;
      }

      const Float T = 1.0 - Rschlick;

      return Kt /** T*/;

      // const auto n = si.n;
      // const auto v = -wo;
      // const bool enters = wo.dot(n) < 0;

      // const Float cosThetaI = v.dot(n);
      // const Float sinThetaI = std::sqrt(std::max(0.0, 1.0 - cosThetaI * cosThetaI));

      // const Float n1 = (enters) ? 1.0 : 1.5;
      // const Float n2 = (enters) ? 1.5 : 1.0;
      // const Float f = n1/n2;

      // const Float sinThetaT = f * sinThetaI;
      // if (sinThetaT > 1.0) {
      //   wi = v - n * 2 * (v.dot(n)); // <---
      //   return Kt;
      // }
      // const Float cosThetaT = std::sqrt(std::max(0.0, 1.0 - sinThetaT * sinThetaT));

      // wi = v * f + n * (f * cosThetaI - cosThetaT);

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
      } else if (sample < ps) {
        e = 1;
        return fr_specular(si, wo, wi) * (1.0 / ps);
      } else if (sample < pt) {
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

  private:
    Direction Kd, Ks, Kt, Ke;
    Float eta;
};

// enum BRDFType {
// BRDF_DIFFUSE,
// BRDF_SPECULAR,
// BRDF_REFRACTION,
// };

// class BRDF {
//   public:
//     explicit BRDF(BRDFType brdf_type) : type{brdf_type} {}
//     virtual Direction f(const Direction &wo, const Direction &wi) const = 0;
//     virtual Direction sample_f(const Direction &wo, Direction &wi) const = 0;
//   public:
//     BRDFType type;
// };

// class DiffuseBRDF : public BRDF {
//   public:
//     explicit DiffuseBRDF(Float Kd) : BRDF(BRDF_DIFFUSE), kd{Kd} {}

//     Direction f(const Direction &wo, const Direction &wi) const override {
//       (void)wo; (void)wi;
//       Float fr = kd / M_PI;
//       return Direction(fr, fr, fr);
//     }
//     Direction sample_f(const Direction &wo, Direction &wi) const override { return f(wo, wi); }

//   private:
//     Float kd;
// };

// class SpecularBRDF : public BRDF {
//   public:
//     SpecularBRDF(BRDFType brdf_type = BRDF_SPECULAR) : BRDF(brdf_type) {}

//     Direction f(const Direction &wo, const Direction &wi) const override {
//       (void)wo; (void)wi;
//       return Direction(0,0,0);
//     }

//     Direction sample_f(const Direction &wo, Direction &wi) const override {
//       return f(wo, wi);
//     }
// };

// struct Material { // TODO
//   Direction color;
//   std::shared_ptr<BRDF> brdf;

//   // TODO: test
//   Float ni = 1;

//   Direction Le;

//   Material(const Direction &c, std::shared_ptr<BRDF> mBRDF, const Direction &light = Direction(0, 0, 0)) : color{c}, brdf{mBRDF}, Le{light} {}
// };

#endif // MATERIAL_H_
