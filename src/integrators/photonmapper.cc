#include "photonmapper.hh"
#include <chrono>
#include <list>
#include "../utils/time.hh"

namespace kernel {
  Float box(Float /*distance*/, Float rk) { return /*M_1_PI * */ (1.0 / (rk * rk)); }
  // TODO: fix
  // Float cone(Float distance, Float rk) {
  //   return std::max(0.0, 1.0 - distance / rk);
  // }
  // Float gaussian(Float distance, Float rk) {
  //   return std::exp(-distance * distance / (2 * rk * rk));
  // }
}

namespace photonmapper {
  std::list<Photon> randomWalk(const Ray &r, const Scene &scene, const Flux &flux, size_t depth, HemisphereSampler sampler) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    std::list<Photon> photons;

    if (depth == 0) return photons;

    SurfaceInteraction interact, tmp;

    if (!scene.intersect(r, interact)) return photons;

    const Point x = interact.p;
    const Direction n = interact.n;
    const Direction wo = interact.wo;

    const auto brdf = interact.material->sampleFr(interact);
    if (brdf == nullptr) return photons; // Absorption

    Direction wi;
    const Spectrum Fr = brdf->sampleFr(sampler, interact, wi);
    const Float cosThetaI = brdf->cosThetaI(sampler, wi, n);
    const Float p = brdf->p(sampler, wi);

    if (!brdf->isDelta) {
      photons.push_back(Photon(x, wo, flux, Fr * cosThetaI / p));
      // TODO: Fuera o dentro
      photons.splice(photons.end(), randomWalk(Ray(x + wi * eps, wi), scene, flux * Fr * cosThetaI / p, depth - 1, sampler));
    }

    return photons;
  }

  Spectrum Li(const Ray &r, const Scene &scene, const PhotonMap &photonMap, ulong k, Float rk, size_t depth, HemisphereSampler sampler) {
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
      return Li(Ray(x + wi * eps, wi), scene, photonMap, k, rk, depth - 1, sampler) * Fr * cosThetaI / p;

    Spectrum L;
    auto nearest = photonMap.nearest_neighbors(x, k, rk);
    for (const Photon *photon : nearest) {
      const Float distance = (x - photon->pos).norm();
      L += Fr * cosThetaI / p * photon->flux * kernel::box(distance, rk);
    }
    return L;
  }

  void render(std::shared_ptr<Camera> &camera, const Scene &scene, size_t spp, size_t maxDepth, HemisphereSampler sampler) {
    const size_t width = camera->film.getWidth();
    const size_t height = camera->film.getHeight();

    auto start = std::chrono::high_resolution_clock::now();

    std::list<Photon> photons;

    // TODO: prepare light sources

    size_t iter = 0;

    // Parameters:
    size_t S = 9999999;
    S = 1000000;
    unsigned long k = 100000;
    float rk = 0.07;

    for (const auto &light : scene.lights) { // puntuales, TODO para luces de area (triangulos)
      #pragma omp parallel for
      for (size_t s = 0; s < S; s++) {
        // TODO: P mal
        const Float theta = std::acos(2 * uniform(0, 1) - 1);
        const Float phi = 2 * M_PI * uniform(0, 1);

        const Flux flux = light.power * 4.0 * M_PI / S;

        const Direction wi = Direction(std::sin(theta) * std::cos(phi),
                                       std::sin(theta) * std::sin(phi),
                                       std::cos(theta));
        const Ray ray(light.p, wi); 

        auto p = randomWalk(ray, scene, flux, maxDepth, sampler);
        #pragma omp critical
        {
          photons.splice(photons.end(), p);
          std::cout << "Creando photon map: " << (float)iter++ * 100.0f / ((float)S) << "%" << std::endl;
        }
      }
    }

    PhotonMap photonMap(photons, PhotonAxisPositition());

    iter = 0;

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
          L += Li(r, scene, photonMap, k, rk, maxDepth, sampler);

          #pragma omp critical
          {
            iter++;
          }
        }
        L /= spp;

        camera->writeColor(i, j, L);
        camera->writeNormal(i, j, si.n);
        camera->writeDepth(i, j, si.t);

        #pragma omp critical
        {
          std::cout << "Rendering: " << (float)iter * 100.0f / ((float)width*height*spp) << "%" << std::endl;
        }
      }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    // TODO: mas info
    std::cout << "[PHOTONMAPPER " << width << "x" << height << "px " << spp << "spp] render took: " << utils::time::format(duration) << std::endl << std::endl;
  }
}
