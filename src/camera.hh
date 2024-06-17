#ifndef CAMERA_H_
#define CAMERA_H_

#include "image/film.hh"
#include "geometry.hh"
#include "ver.hh"

class Camera {
  public:

    Camera(size_t width, size_t height,
           const Point &eye_, const Direction &left_, const Direction &up_, const Direction &forward_,
           size_t color_res = 10000)
      : film(width, height, color_res),
        nFilm(width, height, color_res),
        dFilm(width, height, color_res),
        eye(eye_), left(left_), up(up_), forward(forward_),
        aspectRatio(static_cast<Float>(width) / static_cast<Float>(height)),
        delta_u(2.0 / static_cast<Float>(width)), delta_v(2.0 / static_cast<Float>(height)) {

      assert(left.dot(up) == 0, "left and up must be orthogonal");
      assert(left.dot(forward) == 0, "left and forward must be orthogonal");
      assert(up.dot(forward) == 0, "up and forward must be orthogonal");

      assert(left.norm() == 1, "left must be normalized");
      assert(up.norm() == 1, "up must be normalized");
        
      if (width > height)
        up /= aspectRatio;
      else
        left *= aspectRatio;
    }

    Camera(size_t width, size_t height,
           const Point &eye_, const Point &lookAt, Float focalLength,
           size_t color_res = 10000)
      : Camera(width, height,
               eye_,
               (lookAt - eye_).cross(Direction(0, 1, 0)).normalize(),
               (lookAt - eye_).cross(Direction(0, 1, 0)).cross(lookAt - eye_).normalize(),
               (lookAt - eye_).normalize() * focalLength, color_res
               ) {}
    
    virtual Ray getRay(size_t x, size_t y, uint seed = 5489u) const = 0;

    virtual void writeColor(size_t x, size_t y, const Direction &color) {
      assert(y < film.getWidth(), "x < width");
      assert(x < film.getHeight(), "y < height");

      assert(color.x >= 0, "x >= 0");
      assert(color.y >= 0, "y >= 0");
      assert(color.z >= 0, "z >= 0");

      const size_t idx = y * film.getHeight() * aspectRatio + x;

      image::Pixel &px = film[idx];

      px.r += color.x;
      px.g += color.y;
      px.b += color.z;
    }

    virtual void writeNormal(size_t x, size_t y, const Direction &normal) {
      assert(x < film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      const size_t idx = y * film.getHeight() * aspectRatio + x;

      image::Pixel &px = nFilm[idx];

      px.r = std::abs(normal.x);
      px.g = std::abs(normal.y);
      px.b = std::abs(normal.z);
    }
    
    virtual void writeDepth(size_t x, size_t y, Float depth) {
      assert(x < film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      assert(depth >= 0, "depth < 0");

      const size_t idx = y * film.getHeight() * aspectRatio + x;

      image::Pixel &px = dFilm[idx];

      px.r = depth;
      px.g = depth;
      px.b = depth;
    }

  public: // Portected
    image::Film film;
    image::Film nFilm, dFilm; // normal and depth

    Point eye;
    Direction left, up, forward;

    Float aspectRatio;
    Float delta_u, delta_v;
};

class PinholeCamera : public Camera {
  public:
    using Camera::Camera;

    Ray getRay(size_t x, size_t y, uint seed) const override {
      assert(x <= film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      // Add a random number to the pixel to avoid aliasing
      const Float su = uniform(0, delta_u, seed);
      const Float sv = uniform(0, delta_v, seed);

      // 0, 0 is the top left corner
      const Float u = x / static_cast<Float>(film.getWidth()) + su;
      const Float v = y / static_cast<Float>(film.getHeight()) + sv;

      const Direction d = forward + left * (1 - 2 * u) + up * (1 - 2 * v);

      return Ray(eye, d.normalize());
    }
};

class OrthographicCamera : public Camera {
  public:
    using Camera::Camera;

    Ray getRay(size_t x, size_t y, uint seed) const override {
      assert(x <= film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      // Add a random number to the pixel to avoid aliasing
      const Float su = uniform(0, delta_u, seed);
      const Float sv = uniform(0, delta_v, seed);

      // 0, 0 is the top left corner
      const Float u = x / static_cast<Float>(film.getWidth()) + su;
      const Float v = y / static_cast<Float>(film.getHeight()) + sv;

      const Direction d = forward + left * (1 - 2 * u) + up * (1 - 2 * v);

      return Ray(eye + d, forward.normalize());
    }
};

#endif // CAMERA_H_
