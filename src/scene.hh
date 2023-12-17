#ifndef SCENE_H_
#define SCENE_H_

#include "ver.hh"
#include "geometry.hh"
#include "interaction.hh"
#include "lights.hh"
#include "primitive.hh"
#include <vector>

#include "spectrum.hh"

class Scene {
  public:
    Scene() = default;

    bool intersect(const Ray &r, SurfaceInteraction &interact) const {
      SurfaceInteraction surfInt, tmpSurfInt;
      surfInt.t = std::numeric_limits<Float>::max();
      tmpSurfInt.t = std::numeric_limits<Float>::max();
      bool hit = false;

      for (const auto &primitive : scene) {
        if (primitive->intersect(r, tmpSurfInt))
          if (tmpSurfInt.t < surfInt.t) {
            hit = true;
            surfInt = tmpSurfInt;
          }
      }

      if (hit)
        interact = surfInt;

      return hit;
    }

    Spectrum directLight(const SurfaceInteraction &interact) const {
      constexpr Float eps = 1e-4;

      Spectrum L;
      const Point x = interact.p;
      const Direction n = interact.n;

      for (const auto &light : lights) {
        const Direction wi = (light.p - x).normalize();
        const Float d2l = (light.p - x).norm();

        SurfaceInteraction interact2;
        if (intersect(Ray(x + wi * eps, wi), interact2)) {
          if (interact2.t > d2l - eps) {
            L += light.power / (d2l*d2l) * std::abs(n.dot(wi));
          }
        } else {
          L += light.power / (d2l*d2l) * std::abs(n.dot(wi));
        }
      }

      return L;
    }

    void add(std::unique_ptr<Primitive> primitive) { scene.push_back(std::move(primitive)); }
    void add(const LightPoint &light) { lights.push_back(light); }

  public:
    std::vector<std::unique_ptr<Primitive>> scene;
    std::vector<LightPoint> lights;
};

#endif // SCENE_H_
