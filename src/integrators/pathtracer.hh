#ifndef PATHTRACER_H_
#define PATHTRACER_H_

#include "materials/slides.hh"
#include "camera.hh"
#include "scene.hh"

namespace pathtracer {
  void render(Camera &camera, const Scene &scene, size_t spp, size_t maxDepth, HemisphereSampler sampler = COSINE);
} // namespace pathtracer

#endif // PATHTRACER_H_
