#include "pathtracer.hh"
#include <chrono>

namespace pathtracer {
  Spectrum Li(const Ray &r, const Scene &scene, size_t depth) {
    constexpr Float eps = 1e-4; // Self-shadow eps

    SurfaceInteraction interact;

    if (depth == 0) return Spectrum();
    if (!scene.intersect(r, interact)) return Spectrum();


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

    return Lp + brdf * Li(Ray(x + wi * eps, wi), scene, depth - 1);
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
