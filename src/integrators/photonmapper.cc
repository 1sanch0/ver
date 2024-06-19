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
    // https://graphics.stanford.edu/courses/cs348b-00/course8.pdf
    // Section 3.2.1
    public:
      Cone(Float k_ = 1) : k(k_) { assert(k >= 1, "Cone kernel filter constant k, must be >= 1"); }

      Float operator ()(Float distance, Float rk) const override {
        const Float w_pc = std::max(0.0, 1.0 - distance / (k * rk));
        const Float a = (1.0 - 2.0 / (3.0 * k)) * M_PI * rk * rk;
        return w_pc / a;
      }

    private:
      Float k;
  };

  class Gaussian : public Kernel {
    // https://graphics.stanford.edu/courses/cs348b-00/course8.pdf
    // Section 3.2.2
    public:
      Gaussian(Float alpha = 0.918, Float beta = 1.953) : a(alpha), b(beta) {}

      Float operator ()(Float distance, Float rk) const override {
        return a * (1.0 - (1.0 - std::exp(-b * distance * distance / (2.0 * rk * rk))) / (1.0 - std::exp(-b)));
      }

    private:
      Float a, b;
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
        flux *= Fr * cosThetaI / p;
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

  Spectrum Li(const Ray &r, const Scene &scene, const PhotonMap &globalMap, const PhotonMap &causticMap, ulong k, Float rk, size_t depth, HemisphereSampler sampler, bool nextEventEstimation, const kernel::Kernel &kernel) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    SurfaceInteraction interact;

    if (depth == 0) return Spectrum();
    if (!scene.intersect(r, interact)) return scene.envMapValue(r);

    const Point x = interact.p;
    const Direction n = interact.n;

    const Spectrum Le = interact.material->Le();
    if (Le.max() != 0) return Le; // Material emits

    const auto brdf = interact.material->sampleFr(interact);
    if (brdf == nullptr) return Spectrum(); // Absorption

    Direction wi;
    const Spectrum Fr = brdf->sampleFr(sampler, interact, wi);
    const Float cosThetaI = brdf->cosThetaI(sampler, wi, n);
    const Float p = brdf->p(sampler, wi);

    if (brdf->isDelta)
      return Li(Ray(x + wi * eps, wi), scene, globalMap, causticMap, k, rk, depth - 1, sampler, nextEventEstimation, kernel);

    Spectrum L;
    auto nearest = globalMap.nearest_neighbors(x, k, rk);
    for (const Photon *photon : nearest) {
      const Float distance = (x - photon->pos).norm();
      // if in same hemisphere
      if (n.dot(photon->wi) > 0) {
        L += photon->flux * brdf->fr(interact, wi) * kernel(distance, rk);
      }
    }

    nearest = causticMap.nearest_neighbors(x, k, rk);
    for (const Photon *photon : nearest) {
      const Float distance = (x - photon->pos).norm();
      // if in same hemisphere
      if (n.dot(photon->wi) > 0) {
        L += photon->flux * brdf->fr(interact, wi) * kernel(distance, rk);
      }
    }

    if (nextEventEstimation) {
      const Spectrum Lp = scene.directLight(interact, brdf);
      L += Lp; // TODO: BIEN?
    }

    return L;
  }

  void render(std::shared_ptr<Camera> &camera, const Scene &scene, size_t spp, size_t maxDepth,
              size_t nRandomWalks, unsigned long k, float rk, bool nextEventEstimation, 
              HemisphereSampler sampler) {
    const size_t width = camera->film.getWidth();
    const size_t height = camera->film.getHeight();

    if (scene.lights.empty())
      throw std::runtime_error("No PointLights in scene (required for photon mapping)");

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

    uint seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    // TODO: area lights????
    for (size_t i = 0; i < scene.lights.size(); i++) {
      const auto &light = scene.lights[i];
      const size_t n = nPhotons[i];

      #pragma omp parallel for
      for (size_t s = 0; s < n; s++) {
        // TODO: P mal
        const Float theta = std::acos(2 * uniform(0, 1, seed) - 1);
        const Float phi = 2 * M_PI * uniform(0, 1, seed);

        const Flux flux = light.power * 4.0 * M_PI / n;

        const Direction wi = Direction(std::sin(theta) * std::cos(phi),
                                       std::sin(theta) * std::sin(phi),
                                       std::cos(theta));
        const Ray ray(light.p, wi); 

        auto [caustic, global] = randomWalk2(ray, scene, flux, maxDepth, sampler, !nextEventEstimation);
        #pragma omp critical
        {
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
          L += Li(r, scene, photonMap, photonMap2, k, rk, maxDepth, sampler, nextEventEstimation, kernel::Cone());

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
