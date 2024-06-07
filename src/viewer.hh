#ifndef VIEWER_H_
#define VIEWER_H_
#ifdef VIEWER

#include "scene.hh"
#include "ver.hh"
#include "integrators/pathtracer.hh"

namespace raylib {
  #include "raylib.h"
  #include "raymath.h"
}

class Viewer {
  public:
    Viewer(Scene &scene_, size_t max_depth = 42, HemisphereSampler sampler_ = COSINE);

    virtual ~Viewer();

    void run(); 

  private:
    enum class Mode {
      IMAGE,
      GAME
    };

    Scene &scene;
    size_t maxDepth;
    HemisphereSampler sampler;

    Mode mode;

    size_t idx;
    size_t spp;

    raylib::Camera2D camera;
    raylib::RenderTexture2D target;
    raylib::Color *buffer;

    float dstWidth, dstHeight;
    float dstXOffset, dstYOffset;
    float aspectRatio;

    Direction resetFront, resetLeft, resetUp;
    Point resetEye;
  
  private:
    void render();
    void tonemap(image::Film &film);
    void handleInput();
    void DrawHUD();
    void resetCamera2D();
    void resetTarget();
};

#endif
#endif // VIEWER_H_