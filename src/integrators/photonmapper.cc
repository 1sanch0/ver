#include "photonmapper.hh"
#include <chrono>
#include <list>

namespace kernel {
  Float box(Float distance, Float rk) { return /*M_1_PI * */ (1.0 / (rk * rk)); }
  Float cone(Float distance, Float rk) {
    return 0;
  }
  Float gaussian(Float distance, Float rk) {
    return 0;
  }
}

namespace photonmapper {
  std::list<Photon> randomWalk(const Ray &r, const Scene &scene, const Flux &flux, size_t N) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    std::list<Photon> photons;

    if (N == 0) return photons;

    SurfaceInteraction interact, tmp;

    if (scene.intersect(r, interact)) {
      const Direction wo = interact.wo;
      const Point x = interact.p;

      Direction wi;
      uint e;
      bool absorption;
      #if 0
      const auto brdf = interact.material->fr_sample(interact, wo, wi, absorption, e);
      
      if (absorption)
        return photons;

      const Ray ray(x + wi * eps, wi);

      if (e == 0) // diffuse
        photons.push_back(Photon(x, wo, flux, brdf));

      photons.splice(photons.end(), randomWalk(ray, scene, flux * brdf, N - 1));
      #endif
    }

    return photons;
  }

  Spectrum Li(const Ray &r, const Scene &scene, const PhotonMap &photonMap, ulong k, Float rk, size_t depth) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    SurfaceInteraction interact;

    if (depth == 0) return Spectrum();
    if (!scene.intersect(r, interact)) return Spectrum();

    Spectrum L;

    #if 0
    const Direction wo = interact.wo;
    const Point x = interact.p;
    Direction wi;

    bool absorption; // TODO: tiene sentido hacerlo en los dos sition

    uint e;
    const auto brdf = interact.material->fr_sample(interact, wo, wi, absorption, e);
    if (absorption) return L; // TODO: ver
      
    if (e == 1 || e == 2) // deltas
      return Li(Ray(x + wi * eps, wi), scene, photonMap, k, rk, depth - 1);

    auto nearest = photonMap.nearest_neighbors(x, k, rk);
    for (const Photon *photon : nearest) {
      const Float distance = (x - photon->pos).norm();
      L += brdf * photon->flux * kernel::box(distance, rk);
    }
    #endif
    return L;
  }

  void render(Camera &camera, const Scene &scene, size_t N) {
    const size_t width = camera.film.getWidth();
    const size_t height = camera.film.getHeight();

    auto start = std::chrono::high_resolution_clock::now();

    std::list<Photon> photons;

    // TODO: prepare light sources

    size_t iter = 0;

    // Parameters:
    size_t S = 9999999;
    S = 1000000;
    size_t spp = 12;
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

        auto p = randomWalk(ray, scene, flux, N);
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
        Spectrum L;
        for (size_t s = 0; s < spp; s++) {
          Ray r = camera.getRay(i, j);

          if (scene.intersect(r, si)) {
            L += Li(r, scene, photonMap, k, rk, 16 /*TODO*/);
          }

          #pragma omp critical
          {
            iter++;
          }
        }
        L /= spp;

        camera.writeColor(i, j, L);
        camera.writeNormal(i, j, si.n);
        //cam.writeDepth(i, j, si.t);

        #pragma omp critical
        {
          std::cout << "Rendering: " << (float)iter * 100.0f / ((float)width*height*spp) << "%" << std::endl;
        }
      }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "PhotonMapping render took: " << duration.count() << " milliseconds" << std::endl;
  }
}
