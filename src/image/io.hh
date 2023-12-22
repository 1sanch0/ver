#ifndef IO_H_
#define IO_H_

#include "film.hh"
#include <string>

namespace image {
  [[nodiscard]] Film read(const std::string &filename);
  void write(const std::string &filename, const Film &buffer);
} // namespace image

#endif // IO_H_
