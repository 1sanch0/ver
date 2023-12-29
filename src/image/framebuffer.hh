#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include "ver.hh"
#include <vector>

namespace image {
  class Pixel {
    public:
      Pixel();
      explicit Pixel(Float val);
      Pixel(Float red, Float green, Float blue);

      inline void isValid() const {
        assert(!std::isnan(r), "Pixel red value is NaN");
        assert(!std::isinf(r), "Pixel red value is infinite");
        assert(r >= 0.0, "Pixel red value is negative");
        assert(!std::isnan(g), "Pixel green value is NaN");
        assert(!std::isinf(g), "Pixel green value is infinite");
        assert(g >= 0.0, "Pixel green value is negative");
        assert(!std::isnan(b), "Pixel blue value is NaN");
        assert(!std::isinf(b), "Pixel blue value is infinite");
        assert(b >= 0.0, "Pixel blue value is negative");
      }

      inline Float luminance() const { // formula (3) https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1359728
        return 0.2125 * r + 0.7154 * g + 0.0721 * b; // TODO: y formula (7)
      }

      Float& operator [](size_t idx);
      [[nodiscard]] Float operator [](size_t idx) const;

      Pixel operator +(const Pixel &other) const;
      Pixel& operator +=(const Pixel &other);

      Pixel operator /(Float scalar) const;
      Pixel& operator /=(Float scalar);

    public:
      Float r, g, b;
  };

  class Framebuffer {
    public:
      Framebuffer(size_t width, size_t height);

      Pixel get(size_t i, size_t j) const;

      Pixel& operator [](size_t idx);
      [[nodiscard]] Pixel operator [](size_t idx) const;

      Framebuffer operator +(const Framebuffer &other) const;
      Framebuffer& operator +=(const Framebuffer &other);

      Framebuffer operator /(Float scalar) const;
      Framebuffer& operator /=(Float scalar);

      size_t getWidth() const;
      size_t getHeight() const;

    private:
      size_t rows, cols;
      std::vector<Pixel> buffer;
  };
}

#endif // FRAMEBUFFER_H_
