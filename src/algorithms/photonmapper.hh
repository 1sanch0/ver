#ifndef PHOTONMAPPER_H_
#define PHOTONMAPPER_H_

#include "ver.hh"
#include "geometry.hh"
#include "accelerators/kdtree.h"

#include "camera.hh"
#include "scene.hh"
#include "material.hh"
#include "primitive.hh"
#include "lights.hh"

typedef Direction Flux; // TODO

namespace photonmapper {
  class Photon {
    public:
      Photon(const Point &x, const Direction &wi_, const Flux &flux_, const Spectrum &brdf_)
        : pos{x}, wi{wi_}, flux{flux_}, brdf{brdf_} {}

      Float position(size_t i) const { return pos[i]; }

    public:
      Point pos;
      Direction wi;
      Flux flux;
      Spectrum brdf;
  };

  struct PhotonAxisPositition {
    Float operator()(const Photon& p, size_t i) const {
      return p.position(i);
    }
  };

  using PhotonMap = nn::KDTree<Photon, 3, PhotonAxisPositition>;


  // Spectrum Li(const Ray &r, const Scene &scene, size_t depth);
  void render(Camera &camera, const Scene &scene, size_t N);
} // namespace photonmapper

#endif // PHOTONMAPPER_H_
