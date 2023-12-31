#include "framebuffer.hh"

namespace image {
  Pixel::Pixel() : r{0.0}, g{0.0}, b{0.0} {}

  Pixel::Pixel(Float val)
    : r{val}, g{val}, b{val} {
    isValid();
  }

  Pixel::Pixel(Float red, Float green, Float blue)
    : r{red}, g{green}, b{blue} {
    isValid();
  }

  Float& Pixel::operator [](size_t idx) {
    assert(idx < 3, "Index out of bounds for pixel");
    switch (idx) {
      case 0: return r;
      case 1: return g;
      case 2:
      default: return b;
    }
  }

  Float Pixel::operator [](size_t idx) const {
    assert(idx < 3, "Index out of bounds for pixel");
    switch (idx) {
      case 0: return r;
      case 1: return g;
      case 2:
      default: return b;
    }
  }

  Pixel Pixel::operator +(const Pixel &other) const {
    return Pixel{r + other.r, g + other.g, b + other.b};
  }

  Pixel& Pixel::operator +=(const Pixel &other) {
    r += other.r;
    g += other.g;
    b += other.b;
    isValid();
    return *this;
  }

  Pixel Pixel::operator /(Float scalar) const {
    return Pixel{r / scalar, g / scalar, b / scalar};
  }

  Pixel& Pixel::operator /=(Float scalar) {
    r /= scalar;
    g /= scalar;
    b /= scalar;
    isValid();
    return *this;
  }

  Framebuffer::Framebuffer(size_t width, size_t height)
    : rows{height}, cols{width}, buffer{height * width} {
    assert(width > 0, "Width must be greater than 0");
    assert(height > 0, "Height must be greater than 0");
  }

  Pixel Framebuffer::get(size_t i, size_t j) const {
    assert(i < cols, "Index out of bounds for framebuffer");
    assert(j < rows, "Index out of bounds for framebuffer");
    return buffer[i + j * cols];
  }

  Pixel& Framebuffer::operator [](size_t idx) {
    assert(idx < rows * cols, "Index out of bounds for framebuffer");
    return buffer[idx];
  } 

  Pixel Framebuffer::operator [](size_t idx) const {
    assert(idx < rows * cols, "Index out of bounds for framebuffer");
    return buffer[idx];
  }

  Framebuffer Framebuffer::operator +(const Framebuffer &other) const {
    assert(rows == other.rows, "Framebuffers must have the same number of rows");
    assert(cols == other.cols, "Framebuffers must have the same number of columns");
    Framebuffer result(cols, rows);
    for (size_t i = 0; i < rows * cols; i++)
      result[i] = (*this)[i] + other[i];
    return result;
  }

  Framebuffer& Framebuffer::operator +=(const Framebuffer &other) {
    assert(rows == other.rows, "Framebuffers must have the same number of rows");
    assert(cols == other.cols, "Framebuffers must have the same number of columns");
    for (size_t i = 0; i < rows * cols; i++)
      (*this)[i] += other[i];
    return *this;
  }

  Framebuffer Framebuffer::operator /(Float scalar) const {
    assert(!std::isnan(scalar), "Cannot divide by NaN");
    assert(!std::isinf(scalar), "Cannot divide by infinity");
    assert(scalar > 0.0, "Pixels cannot be divided by negative number or 0");
    Framebuffer result(cols, rows);
    for (size_t i = 0; i < rows * cols; i++)
      result[i] = (*this)[i] / scalar;
    return result;
  }

  Framebuffer& Framebuffer::operator /=(Float scalar) {
    assert(!std::isnan(scalar), "Cannot divide by NaN");
    assert(!std::isinf(scalar), "Cannot divide by infinity");
    assert(scalar > 0.0, "Pixels cannot be divided by negative number or 0");
    for (size_t i = 0; i < rows * cols; i++)
      (*this)[i] /= scalar;
    return *this;
  }

  size_t Framebuffer::getWidth() const { return cols; }
  size_t Framebuffer::getHeight() const { return rows; }
}