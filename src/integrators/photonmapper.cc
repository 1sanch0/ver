#include "photonmapper.hh"
#include <chrono>
#include <list>
#include "../utils/time.hh"

#include "../utils/lwpb.hh"

namespace kernel {
  class Kernel {
    public:
      virtual Float operator ()(Float distance, Float rk) const = 0;
  };

  class Box : public Kernel {
    public:
      Float operator ()(Float, Float rk) const override {
        return M_1_PI * (1.0 / (rk * rk));
      }
  };

  class Cone : public Kernel {
    public:
      Float operator ()(Float distance, Float rk) const override {
        return std::max(0.0, 1.0 - distance / rk);
      }
  };

  class Gaussian : public Kernel {
    public:
      Float operator ()(Float distance, Float rk) const override {
        return std::exp(-distance * distance / (2 * rk * rk));
      }
  };
}

namespace photonmapper {
  struct PhotonMaps {
    PhotonMaps() = default;
                               // L = light source, S = specular or transmisive, D = diffuse
    std::list<Photon> caustic; // LS+D
    std::list<Photon> global;  // L{S|D}*D
  };

  PhotonMaps randomWalk2(Ray r, const Scene &scene, Flux flux, size_t depth, HemisphereSampler sampler, bool storeFirst) {
    constexpr Float eps = 1e-4; // Self-shadow eps
    
    PhotonMaps maps;

    SurfaceInteraction interact;

    bool isFirst = true;
    bool isCaustic = true;
    for (size_t i = 0; i < depth; i++) {
      if (!scene.intersect(r, interact)) break;

      const Point x = interact.p;
      const Direction n = interact.n;
      const Direction wo = interact.wo;

      const auto brdf = interact.material->sampleFr(interact);
      if (brdf == nullptr) break; // Absorption

      Direction wi;
      const Spectrum Fr = brdf->sampleFr(sampler, interact, wi);
      const Float cosThetaI = brdf->cosThetaI(sampler, wi, n);
      const Float p = brdf->p(sampler, wi);

      if (brdf->isDelta) {
        // Delta material, just propagate
        r = Ray(x + wi * eps, wi);
        flux *= Fr * cosThetaI / p; // TODO: check if this is correct. This should be 1 no?
      } else {
        const bool store = storeFirst || !isFirst;
        isFirst = false;

        if (store) {
          if (isCaustic) 
            maps.caustic.push_back(Photon(x, wo, flux));
          else
            maps.global.push_back(Photon(x, wo, flux));
        }
        
        isCaustic = false;
        r = Ray(x + wi * eps, wi);
        flux *= Fr * cosThetaI / p;
      }
    }

    return maps;
  }

  std::list<Photon> randomWalk(const Ray &r, const Scene &scene, const Flux &flux, size_t depth, HemisphereSampler sampler, bool first = false) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    std::list<Photon> photons;

    if (depth == 0) return photons;

    SurfaceInteraction interact, tmp;

    if (!scene.intersect(r, interact)) return photons; // TODO: ENV MAP?

    const Point x = interact.p;
    const Direction n = interact.n;
    const Direction wo = interact.wo;

    const auto brdf = interact.material->sampleFr(interact);
    if (brdf == nullptr) return photons; // Absorption

    Direction wi;
    const Spectrum Fr = brdf->sampleFr(sampler, interact, wi);
    const Float cosThetaI = brdf->cosThetaI(sampler, wi, n);
    const Float p = brdf->p(sampler, wi);

    if (!brdf->isDelta) { // Only store photons in non-delta materials
      photons.push_back(Photon(x, wo, flux * Fr * cosThetaI / p));
    }
    photons.splice(photons.end(), randomWalk(Ray(x + wi * eps, wi), scene, flux * Fr * cosThetaI / p, depth - 1, sampler, false));

    return photons;
  }

  Spectrum Li(const Ray &r, const Scene &scene, const PhotonMap &photonMap, ulong k, Float rk, size_t depth, HemisphereSampler sampler, const kernel::Kernel &kernel) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    SurfaceInteraction interact;

    if (depth == 0) return Spectrum();
    if (!scene.intersect(r, interact)) return scene.envMapValue(r); // TODO: bien?

    const Point x = interact.p;
    const Direction n = interact.n;

    // TODO: bien?
    const Spectrum Le = interact.material->Le();
    if (Le.max() != 0) return Le; // Material emits

    const auto brdf = interact.material->sampleFr(interact);
    if (brdf == nullptr) return Spectrum(); // Absorption

    Direction wi;
    const Spectrum Fr = brdf->sampleFr(sampler, interact, wi);
    const Float cosThetaI = brdf->cosThetaI(sampler, wi, n);
    const Float p = brdf->p(sampler, wi);

    if (brdf->isDelta)
      return Li(Ray(x + wi * eps, wi), scene, photonMap, k, rk, depth - 1, sampler, kernel) * Fr * cosThetaI / p;

    Spectrum L;
    auto nearest = photonMap.nearest_neighbors(x, k, rk);
    for (const Photon *photon : nearest) {
      const Float distance = (x - photon->pos).norm();
      // if in same hemisphere
      if (n.dot(photon->wi) > 0) {
        // L += Fr * cosThetaI / p * photon->flux * kernel(distance, rk);
        L += photon->flux * brdf->fr(interact, wi) * kernel(distance, rk);
      }
    }
    // const Spectrum Lp = scene.directLight(interact) * brdf->fr(interact, wi); // M_PI cancels out in brdf->fr
    const Spectrum Lp;
    return Lp + L / (M_PI * rk * rk);
  }

  void render(std::shared_ptr<Camera> &camera, const Scene &scene, size_t spp, size_t maxDepth, HemisphereSampler sampler) {
    const size_t width = camera->film.getWidth();
    const size_t height = camera->film.getHeight();

    // Parameters: (TODO: move to args)
    size_t nRandomWalks = 400000 * 40;
    unsigned long k = 100*10;
    float rk = 0.01f;
    // ----
    // k = 10;
    // nRandomWalks = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    std::list<Photon> photons;
    std::list<Photon> photons2;

    Float totalPower = 0;
    for (const auto &light : scene.lights)
      totalPower += light.power.norm();

    std::vector<size_t> nPhotons(scene.lights.size());
    for (size_t i = 0; i < scene.lights.size(); i++)
      nPhotons[i] = nRandomWalks * std::round(scene.lights[i].power.norm() / totalPower);


    utils::lwpb pbar(nRandomWalks, "Photon Mapping");

    // TODO: area lights????
    for (size_t i = 0; i < scene.lights.size(); i++) {
      const auto &light = scene.lights[i];
      const size_t n = nPhotons[i];

      #pragma omp parallel for
      for (size_t s = 0; s < n; s++) {
        // TODO: P mal
        const Float theta = std::acos(2 * uniform(0, 1) - 1);
        const Float phi = 2 * M_PI * uniform(0, 1);

        const Flux flux = light.power * 4.0 * M_PI / n;

        const Direction wi = Direction(std::sin(theta) * std::cos(phi),
                                       std::sin(theta) * std::sin(phi),
                                       std::cos(theta));
        const Ray ray(light.p, wi); 

        // auto p = randomWalk(ray, scene, flux, maxDepth, sampler, true);
        auto [caustic, global] = randomWalk2(ray, scene, flux, maxDepth, sampler, true);
        #pragma omp critical
        {
          // photons.splice(photons.end(), p);
          photons.splice(photons.end(), global);
          photons2.splice(photons2.end(), caustic);
          pbar.update();
        }
      }
    }

    PhotonMap photonMap(photons, PhotonAxisPositition());
    PhotonMap photonMap2(photons2, PhotonAxisPositition());

    pbar = utils::lwpb(width*height*spp, "Rendering");

    #pragma omp parallel for
    for (size_t i = 0; i < width; i++) {
      for (size_t j = 0; j < height; j++) {
        SurfaceInteraction si;
        si.t = 0;
        si.n = Direction(0, 0, 0);

        Spectrum L;
        for (size_t s = 0; s < spp; s++) {
          Ray r = camera->getRay(i, j);

          scene.intersect(r, si);
          L += Li(r, scene, photonMap, k, rk, maxDepth, sampler, kernel::Gaussian()) * 0.5;
          L += Li(r, scene, photonMap2, k, rk, maxDepth, sampler, kernel::Gaussian()) * 0.5;

          #pragma omp critical
          {
            pbar.step();
          }
        }
        L /= spp;

        camera->writeColor(i, j, L);
        camera->writeNormal(i, j, si.n);
        camera->writeDepth(i, j, si.t);

        #pragma omp critical
        {
          pbar.print();
        }
      }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    // TODO: mas info
    std::cout << "[PHOTONMAPPER " << width << "x" << height << "px " << spp << "spp] render took: " << utils::time::format(duration) << std::endl << std::endl;
  }
}
