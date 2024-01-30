#ifndef PHOTONMAPPER_H_
#define PHOTONMAPPER_H_

#include "ver.hh"
#include "geometry.hh"
#include "kdtree.h"

#include "camera.hh"
#include "scene.hh"
#include "materials/slides.hh"
#include "shapes/primitive.hh"
#include "lights.hh"

typedef Direction Flux; // TODO

namespace photonmapper {
  class Photon {
    public:
      Photon(const Point &x, const Direction &wi_, const Flux &flux_)
        : pos{x}, wi{wi_}, flux{flux_} {}

      Float position(size_t i) const { return pos[i]; }

    public:
      Point pos;
      Direction wi;
      Flux flux;
  };

  struct PhotonAxisPositition {
    Float operator()(const Photon& p, size_t i) const {
      return p.position(i);
    }
  };

  using PhotonMap = nn::KDTree<Photon, 3, PhotonAxisPositition>;

  void render(std::shared_ptr<Camera> &camera, const Scene &scene, size_t spp, size_t maxDepth, HemisphereSampler sampler = COSINE);
} // namespace photonmapper

#endif // PHOTONMAPPER_H_
