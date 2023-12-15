#ifndef FILM_H_
#define FILM_H_

#include "ver.hh"

struct Pixel {
  Float r, g, b;

  [[nodiscard]] inline Float luminance() const { // formula (3) https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1359728
    // TODO: y formula (7)
    return 0.2125 * r + 0.7154 * g + 0.0721 * b;
  }

  Float& operator [](size_t idx) {
    assert(idx < 3, "Index out of bounds for pixel struct");
    switch (idx) {
      case 0: return r;
      case 1: return g;
      case 2:
      default: return b;
    }
  }
};

class Film {
  public:
    Film(size_t width_, size_t height_, size_t color_res);
    Film(const Film &film); // Copy constructor
    // TODO: move constructor

    Pixel& operator [](size_t idx);
    [[nodiscard]] Pixel operator [](size_t idx) const;

    [[nodiscard]] size_t getWidth() const;
    [[nodiscard]] size_t getHeight() const;
    [[nodiscard]] size_t getColorRes() const;
    void setColorRes(Float color_res);

    [[nodiscard]] size_t size() const;

    [[nodiscard]] Float max() const;

    void channelAvg(Float &r, Float &g, Float &b);
    void luminanceStats(Float &avg, Float &min, Float &max);

  private:
    size_t width, height;
    size_t color_resolution;
    std::unique_ptr<Pixel[]> buffer;
};

#endif // FILM_H_
