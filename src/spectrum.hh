#ifndef SPECTRUM_H_
#define SPECTRUM_H_

#include "geometry.hh"

typedef Direction Spectrum; // TODO: make a Spectrum class

inline
Spectrum fromHex(const std::string &hex) {
  std::string num = (hex[0] == '#') ? hex.substr(1) : hex;
  assert(num.size() == 6, "Invalid hex color");

  const Float r = std::stoi(num.substr(0, 2), nullptr, 16) / 255.0;
  const Float g = std::stoi(num.substr(2, 2), nullptr, 16) / 255.0;
  const Float b = std::stoi(num.substr(4, 2), nullptr, 16) / 255.0;

  return Spectrum(r, g, b);
}

inline
Spectrum fromHex(int hex) {
  const Float r = ((hex >> 16) & 0xFF) / 255.0;
  const Float g = ((hex >> 8) & 0xFF) / 255.0;
  const Float b = ((hex) & 0xFF) / 255.0;

  return Spectrum(r, g, b);
}

#endif // SPECTRUM_H_
