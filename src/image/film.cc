#include "film.hh"

namespace image {
  Film::Film(size_t width, size_t height, size_t colorResolution)
    : buffer(width, height), color_resolution{colorResolution} {
    assert(colorResolution > 0, "Color resolution must be positive");
  }

  Pixel& Film::operator [](size_t idx) { return buffer[idx]; }
  Pixel Film::operator [](size_t idx) const { return buffer[idx]; }

  size_t Film::getWidth() const { return buffer.getWidth(); }
  size_t Film::getHeight() const { return buffer.getHeight(); }
  size_t Film::getColorRes() const {return color_resolution; }
  void Film::setColorRes(Float color_res) { color_resolution = color_res; }

  size_t Film::size() const { return buffer.getWidth() * buffer.getHeight(); }

  Float Film::max() const {
    Float max = 0;
    for (size_t i = 0; i < this->size(); i++) {
      Pixel px = (*this)[i];
      if (px.r > max) max = px.r;
      if (px.g > max) max = px.g;
      if (px.b > max) max = px.b;
    }
    return max;
  }
  void Film::channelAvg(Float &r, Float &g, Float &b) {
    r = 0; g = 0; b = 0;
    for (size_t i = 0; i < this->size(); i++) {
      Pixel px = (*this)[i];
      r += px.r;
      g += px.g;
      b += px.b;
    }
    r /= this->size();
    g /= this->size();
    b /= this->size();
  }

  void Film::luminanceStats(Float &avg, Float &min, Float &max) {
    Float L = (*this)[0].luminance();
    avg = L; min = L; max = L;
    for (size_t i = 1; i < this->size(); i++) {
      L = (*this)[i].luminance();

      avg += L;
      if (L < min) min = L;
      if (L > max) max = L;
    }
    avg /= this->size();
  }
}