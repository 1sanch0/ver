#include "tonemap.hh"
#include <cmath>

namespace image {
  namespace tonemap {
    static inline Float Clamp(Float val, Float low, Float high) {
      if (val < low) return low;
      if (val > high) return high;
      return val;
    }

    // TODO: review with all possible tonemappers and maybe reuse code
    // TODO: use LAB, HSV

    Gamma::Gamma(Float gamma_, Float clamp_) :
      invGamma{1 / gamma_}, clamp{clamp_} {}

    void Gamma::applyTo(Film &film) {
      DEBUG_CODE({
        std::cout << "[POSTPROCESS: GAMMA] gamma: " << 1 / invGamma;
        std::cout << ", clamp: " << clamp << std::endl;;
      });
      assert(film.getColorRes() > 255, "Film is already LDR");

      film.setColorRes(255);

      #pragma omp parallel for
      for (size_t i = 0; i < film.size(); i++) {
        Pixel px = film[i];
        px.r = forward(px.r);
        px.g = forward(px.g);
        px.b = forward(px.b);

        film[i] = px;
      }
    }

    Float Gamma::forward(Float x) const {
      Float eq = x / this->clamp;
      Float ceq = Clamp(eq, 0, this->clamp);
      // TODO assert not nan y <1
      return std::pow(ceq, invGamma);
    }

    void Reinhard2002::applyTo(Film &film) {
      DEBUG_CODE({
        std::cout << "[POSTPROCESS: REINHARD2002]" << std::endl;
      });

      assert(film.getColorRes() > 255, "Film is already LDR");
      film.setColorRes(255);

      Float max = film.max();

      #pragma omp parallel for
      for (size_t i = 0; i < film.size(); i++) {
        Pixel px = film[i];

        px.r = forward(px.r, max);
        px.g = forward(px.g, max);
        px.b = forward(px.b, max);

        film[i] = px;
      }
    }

    Float Reinhard2002::forward(Float x, float max) const {
      return (x * (1 + x / (max * max))) / (1 + x);
    }

    Reinhard2005::Reinhard2005(Float intensity, Float adaptation, Float color_correction, Float contrast)
      : f_{intensity}, m_{contrast}, a{adaptation}, c{color_correction} {

      assert((m_ >= 0.3 && m_ < 1) || m_ == 0, "Contrast operating range should be [0.3, 1.0) or 0 for 0.3 + 0.7*k^1.4");
      assert(std::abs(f_) <= 8, "Intensity's operating range should be [-8, 8]");
      assert(color_correction >= 0 && color_correction <= 1, "Color correction's operating range should be [0, 1]");
      assert(adaptation >= 0 && adaptation <= 1, "Light adaptation should be [0, 1]");
    }

    void Reinhard2005::applyTo(Film &film) {
      DEBUG_CODE({
        std::cout << "[POSTPROCESS: REINHARD2005]" << std::endl;
      });

      //assert(film.getColorRes() > 255, "Film is already LDR");
      //film.setColorRes(255);

      Float rAvg, gAvg, bAvg;
      film.channelAvg(rAvg, gAvg, bAvg);
      Float lAvg, lMin, lMax;
      film.luminanceStats(lAvg, lMin, lMax);

      Float Llav = 0;

      #pragma omp parallel for
      for (size_t i = 0; i < film.size(); i++) {
        Llav += std::log(film[i].r + 1e-4);
        Llav += std::log(film[i].g + 1e-4);
        Llav += std::log(film[i].b + 1e-4);
      }

      Llav /= film.size();

      Float f = std::exp(-f_);
      //Float k = (std::log(lMax) - std::log(lAvg)) /
                //(std::log(lMax) - std::log(lMin));
      Float k = (std::log(lMax) - Llav) /
                (std::log(lMax) - std::log(lMin));

      std::cout << Llav << ", " << std::log(lAvg) << ", " << k << std::endl;

      Float m = (m_ > 0) ? m_ : 0.3 + 0.7 * std::pow(k, 1.4);

      Pixel px;
      Float L, I_a, I_g, I_l;

      Float Cav[3] = {rAvg, gAvg, bAvg};

      #pragma omp parallel for
      for (size_t i = 0; i < film.size(); i++) {
        px = film[i];
        L = px.luminance();

        for (size_t j = 0; j < 3; j++) {
          I_l = c * px[j]  + (1 - c) * L;
          I_g = c * Cav[j] + (1 - c) * lAvg;
          I_a = a * I_l    + (1 - a) * I_g;
          px[j] /= px[j] + std::pow(f * I_a, m);
        }

        //I_l = c * px.r + (1 - c) * L;
        //I_g = c * rAvg + (1 - c) * lAvg;
        //I_a = a * I_l  + (1 - a) * I_g;
        //px.r /= px.r + std::pow(f * I_a, m);
  //
        //I_l = c * px.g + (1 - c) * L;
        //I_g = c * gAvg + (1 - c) * lAvg;
        //I_a = a * I_l  + (1 - a) * I_g;
        //px.g /= px.g + std::pow(f * I_a, m);
  //
        //I_l = c * px.b + (1 - c) * L;
        //I_g = c * bAvg + (1 - c) * lAvg;
        //I_a = a * I_l  + (1 - a) * I_g;
        //px.b /= px.b + std::pow(f * I_a, m);

        film[i] = px;
      }

      Float max = film.max(); // TODO: bien?
      Gamma(1, max).applyTo(film);
    }
  }
}
