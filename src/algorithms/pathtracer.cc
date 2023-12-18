#include "pathtracer.hh"
#include <chrono>

namespace pathtracer {
  Spectrum Li(const Ray &r, const Scene &scene, size_t depth) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    HemisphereSampler sampler = COSINE;

    SurfaceInteraction interact;

    if (depth == 0) return Spectrum();
    if (!scene.intersect(r, interact)) return Spectrum();

    const Point x = interact.p;
    const Direction n = interact.n;

    const auto brdf = interact.material->sampleFr();
    if (brdf == nullptr) return Spectrum(); // Absorption

    const Spectrum Le = interact.material->Le();
    if (Le.max() != 0) return Le; // Material emits

    Direction wi;
    const Spectrum L = brdf->sampleFr(sampler, interact, wi);
    const Float cosThetaI = brdf->cosThetaI(sampler, wi, n);
    const Float p = brdf->p(sampler, wi);

    const Spectrum Lp = scene.directLight(interact) * brdf->fr(interact, wi) * M_PI; // * std::abs(wi.dot(n)); // TODO: ASK

    return Lp + L * Li(Ray(x + wi * eps, wi), scene, depth - 1) * cosThetaI / p;

    #if 0
    const Point x = interact.p;
    const Direction wo = interact.wo;
    Direction wi;

    bool absorption;
    uint material_event;

    const auto brdf = interact.material->fr_sample(interact, wo, wi, absorption, material_event);
    if (absorption) return Spectrum();

    const Spectrum Le = interact.material->Le();
    if (Le.max() != 0) return Le; // Material emits

    // Puntual light
    const Spectrum Lp = (material_event == 0) ? brdf * scene.directLight(interact) : Spectrum(); 
    // This lo;; be miltiplay be bdrdf cosite tems (both)

    return Lp + brdf * Li(Ray(x + wi * eps, wi), scene, depth - 1);
    #endif
  }

  void render(Camera &camera, const Scene &scene, size_t maxDepth) {
    const size_t width = camera.film.getWidth();
    const size_t height = camera.film.getHeight();

    auto start = std::chrono::high_resolution_clock::now();

    size_t spp = 128*2;

    size_t iter = 0;
    
    // ----------
    // Ray r(Point(0, 0, -3.5), Direction(0.220884472, -0.163036093, 0.961576521));
    // Li(r, scene, maxDepth);
    // return;
    // ----------

    #pragma omp parallel for
    for (size_t i = 0; i < width; i++) {
      for (size_t j = 0; j < height; j++) {
        SurfaceInteraction si;
        Direction L;
        for (size_t s = 0; s < spp; s++) {
          Ray r = camera.getRay(i, j);

          scene.intersect(r, si);
          L += Li(r, scene, maxDepth);

          #pragma omp critical
          {
            iter++;
          }
        }
        L /= spp;

        camera.writeColor(i, j, L);
        si.n.x = (si.n.x < 0) ? 0 : si.n.x;
        si.n.y = (si.n.y < 0) ? 0 : si.n.y;
        si.n.z = (si.n.z < 0) ? 0 : si.n.z;
        camera.writeNormal(i, j, si.n);
        //cam.writeDepth(i, j, si.t);

        #pragma omp critical
        {
          std::cout << (float)iter * 100.0f / ((float)width*height*spp) << "%" << std::endl;
        }
      }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Pathtracing render took: " << duration.count() << " milliseconds" << std::endl;
  }
}
