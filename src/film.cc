#include "film.hh"

Film::Film(size_t width_, size_t height_, size_t color_res)
  : width{width_}, height{height_}, color_resolution{color_res} {

  this->buffer = std::unique_ptr<Pixel[]>(new Pixel[width * height]);
}
Film::Film(const Film &film)
  : width{film.width}, height{film.height}, color_resolution{film.color_resolution} {
  this->buffer = std::unique_ptr<Pixel[]>(new Pixel[width * height]);

  #pragma omp parallel for
  for (size_t i = 0; i < width * height; i++) // TODO: faster than std::copy?
    this->buffer[i] = film.buffer[i];
}

// TODO: assert non nan

Pixel& Film::operator [](size_t idx) {
  assert(idx < width * height, "Index out of bounds");
//  assert(buffer != nullptr, "Buffer is null");
  return this->buffer[idx];
}

Pixel Film::operator [](size_t idx) const {
  assert(idx < width * height, "Index out of bounds");
//  assert(buffer != nullptr, "Buffer is null");
  return this->buffer[idx];
}

size_t Film::getWidth() const { return width; }
size_t Film::getHeight() const { return height; }
size_t Film::getColorRes() const {return color_resolution; }
void Film::setColorRes(Float color_res) { color_resolution = color_res; }
size_t Film::size() const { return this->width * this->height; }

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
