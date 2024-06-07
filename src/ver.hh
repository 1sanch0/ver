#ifndef VER_H_
#define VER_H_

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(EMSCRIPTEN)
  #define _USE_MATH_DEFINES
  typedef unsigned int uint;
  typedef unsigned long ulong;
#endif

#ifdef NDEBUG
#define DEBUG_CODE(code)
#else
#define DEBUG_CODE(code) code
#endif

#include <cstdlib>
#define assert(expr, msg) DEBUG_CODE({                              \
  if (!(expr)) {                                                    \
    fprintf(stderr, "Assertion failed: %s, in function %s, file %s, line %d\n", #expr, __FUNCTION__, __FILE__, __LINE__); \
    fprintf(stderr, msg);                                           \
    fprintf(stderr, "\n");                                          \
    std::abort();                                                   \
  }                                                                 \
})

#include <iostream>
#include <stdint.h>
#include <stddef.h>
#include <algorithm>
#include <memory>
#include <random>

#ifndef EMSCRIPTEN
#include <omp.h>
#endif


typedef float Float;
//typedef double Float;

inline
Float uniform(Float min, Float max) {
  static thread_local std::mt19937 generator;
  std::uniform_real_distribution<Float> distr(min, max);
  return distr(generator);
}


[[nodiscard]] constexpr Float lerp(Float t, Float v1, Float v2) { return (1 - t) * v1 + t * v2; }
[[nodiscard]] constexpr Float gamma(int n) {
  Float eps = std::numeric_limits<Float>::epsilon() * 0.5;
  return (n * eps) / (1.0 - n * eps);
}

[[nodiscard]] constexpr Float clamp(Float val, Float min, Float max) {
  Float t = (val < min) ? min : val;
  return (t > max) ? max : t;
}

// Returns 1 if val is greater than zero and -1 otherwise
[[nodiscard]] constexpr Float sign(Float val) { return (val > 0) - (val <= 0); }

#endif // VER_H_
