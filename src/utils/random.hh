#ifndef RANDOM_H_
#define RANDOM_H_

#include "ver.hh"
#include "geometry.hh"

namespace utils {
  namespace random {
    // Perlin Noise (https://mrl.cs.nyu.edu/~perlin/noise/)
    Float perlinNoise(const Point &point);
    Float perlinNoise(Float x, Float y, Float z);
  }
}

#endif // RANDOM_H_
