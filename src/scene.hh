#ifndef SCENE_H_
#define SCENE_H_

#include "ver.hh"
#include "geometry.hh"
#include "interaction.hh"
#include "lights.hh"
#include "shapes/primitive.hh"
#include <vector>

#include "spectrum.hh"

#include "accelerators/bvh.hh"

#include "texture.hh"

class EnvironmentMap { // TODO: Review
  public:
    explicit EnvironmentMap(std::shared_ptr<Texture> texture_) : texture(texture_) {}

    Spectrum value(const Ray &r) const {
      if (texture == nullptr) return Spectrum();

      SurfaceInteraction interact;
      Float m = 2.0 * std::sqrt(r.d.x*r.d.x + r.d.y*r.d.y + (r.d.z + 1.0)*(r.d.z + 1.0));
      interact.u = r.d.x / m + 0.5;
      interact.v =  1.0 - (r.d.y / m + 0.5);
      // const Float theta = std::acos(r.d.z);
      // const Float phi = std::atan2(r.d.y, r.d.x);
      // interact.u = phi / (2 * M_PI);
      // interact.v = theta / M_PI;

      return texture->value(interact);
    }

  private:
    std::shared_ptr<Texture> texture;
};

class Scene {
  public:
    Scene() : scene{}, lights{}, envMap(nullptr), camera{nullptr} {};

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
      constexpr Float eps = 5e-4;

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

    void set(const std::shared_ptr<Texture> &env) { envMap = EnvironmentMap(env); }
    void set(const std::shared_ptr<Camera> &cam) { camera = cam; }

    void makeBVH() {
      std::vector<std::shared_ptr<Primitive>> p(scene.size());

      for (size_t i = 0; i < scene.size(); i++)
        p[i] = std::move(scene[i]);
      
      scene.clear();

      scene.push_back(std::make_unique<BVH>(std::move(p)));
    }

    Spectrum envMapValue(const Ray &r) const {
      return envMap.value(r);
    }

  public:
    std::vector<std::unique_ptr<Primitive>> scene;
    std::vector<LightPoint> lights;
    EnvironmentMap envMap;
    std::shared_ptr<Camera> camera;
};

#endif // SCENE_H_
