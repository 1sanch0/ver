#ifndef PATHTRACER_H_
#define PATHTRACER_H_

#include "materials/slides.hh"
#include "camera.hh"
#include "scene.hh"
#include <memory>

namespace pathtracer {
  Spectrum Li(const Ray &r, const Scene &scene, size_t depth, HemisphereSampler sampler);
  void render(std::shared_ptr<Camera> &camera, const Scene &scene, size_t spp, size_t maxDepth, HemisphereSampler sampler = COSINE, uint seed = 5489u);
} // namespace pathtracer

#endif // PATHTRACER_H_
