#ifndef TONEMAP_H_
#define TONEMAP_H_

#include "ver.hh"
#include "film.hh"

namespace tonemap {
  class Tonemap {
    public:
      virtual void applyTo(Film &film) = 0;
  };

  class Gamma : public Tonemap {
    public:
      Gamma(Float gamma_, Float clamp_);
      void applyTo(Film &film) override;
      Float forward(Float x) const;
    private:
      Float invGamma, clamp;
  };

  // https://www-old.cs.utah.edu/docs/techreports/2002/pdf/UUCS-02-001.pdf
  class Reinhard2002 : public Tonemap {
    public:
      void applyTo(Film &film) override;
      Float forward(Float x, Float max) const;
  };

  class Reinhard2005 : public Tonemap {
    public:
      Reinhard2005(Float intensity = 0,
                   Float adaptation = 1,
                   Float color_correction = 0,
                   Float contrast = 0);
      void applyTo(Film &film) override;
    private:
      Float f_; // Overall intensity; higher values will make the result lighter, whereas lower values make the image darker.
      Float m_; // Contrast
      Float a;  // Adaptation       TODO: explain
      Float c;  // Color correction TODO: explain

  };
}

#endif // TONEMAP_H_
