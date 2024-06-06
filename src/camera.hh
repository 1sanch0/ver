#ifndef CAMERA_H_
#define CAMERA_H_

#include "image/film.hh"
#include "geometry.hh"
#include "ver.hh"

class Camera {
  public:

    Camera(size_t width, size_t height, size_t color_res = 10000)
      : film(width, height, color_res),
        nFilm(width, height, color_res),
        dFilm(width, height, color_res),
        aspectRatio(static_cast<Float>(width) / static_cast<Float>(height)),
        delta_u(2.0 / static_cast<Float>(width)), delta_v(2.0 / static_cast<Float>(height)) {}

    virtual Ray getRay(size_t x, size_t y) const = 0;
    virtual void writeColor(size_t x, size_t y, const Direction &color) = 0;
    virtual void writeNormal(size_t x, size_t y, const Direction &normal) = 0;
    virtual void writeDepth(size_t x, size_t y, Float depth) = 0;

  public: // Portected
    image::Film film;
    image::Film nFilm, dFilm; // normal and depth

    Float aspectRatio;
    Float delta_u, delta_v;
};

class PinholeCamera : public Camera {
  public:
    PinholeCamera(size_t width, size_t height, const Point &eye_, const Direction &left_, const Direction &up_, const Direction &forward_)
      : Camera(width, height), eye(eye_), left(left_), up(up_), forward(forward_) {
        // TODO: Check orthogonality and normalization

        if (width > height)
          up /= aspectRatio;
        else
          left *= aspectRatio;
      }

    PinholeCamera(size_t width, size_t height, const Point &eye_, const Point &lookAt, Float focalLength)
      : Camera(width, height), eye(eye_), left(), up(), forward() {
        const Direction look = (lookAt - eye).normalize();
        left = look.cross(Direction(0, 1, 0)).normalize();
        up = left.cross(look).normalize();
        forward = look * focalLength;
        
        if (width > height)
          up /= aspectRatio;
        else
          left *= aspectRatio;
      }

    Ray getRay(size_t x, size_t y) const override {
      assert(x <= film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      // Add a random number to the pixel to avoid aliasing
      const Float su = uniform(0, delta_u);
      const Float sv = uniform(0, delta_v);

      // 0, 0 is the top left corner
      const Float u = x / static_cast<Float>(film.getWidth()) + su;
      const Float v = y / static_cast<Float>(film.getHeight()) + sv;

      const Direction d = forward + left * (1 - 2 * u) + up * (1 - 2 * v);

      return Ray(eye, d.normalize());
    }

    void writeColor(size_t x, size_t y, const Direction &color) override {
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

    void writeNormal(size_t x, size_t y, const Direction &normal) override {
      assert(x < film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      const size_t idx = y * film.getHeight() * aspectRatio + x;

      image::Pixel &px = nFilm[idx];

      px.r = std::abs(normal.x);
      px.g = std::abs(normal.y);
      px.b = std::abs(normal.z);
    }

    void writeDepth(size_t x, size_t y, Float depth) override {
      assert(x < film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      assert(depth >= 0, "depth < 0");

      const size_t idx = y * film.getHeight() * aspectRatio + x;

      image::Pixel &px = dFilm[idx];

      px.r = depth;
      px.g = depth;
      px.b = depth;
    }

  public: // Private
    Point eye;
    Direction left, up, forward;
};

class OrthographicCamera : public PinholeCamera { // TODO: IMPROVE and inherit from Camera
  public:

    OrthographicCamera(size_t width, size_t height, const Point &eye_, const Direction &left_, const Direction &up_, const Direction &forward_)
      : PinholeCamera(width, height, eye_, left_, up_, forward_) {}

    Ray getRay(size_t x, size_t y) const override {
      assert(x <= film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      // Add a random number to the pixel to avoid aliasing
      const Float su = uniform(0, delta_u);
      const Float sv = uniform(0, delta_v);

      // 0, 0 is the top left corner
      const Float u = x / static_cast<Float>(film.getWidth()) + su;
      const Float v = y / static_cast<Float>(film.getHeight()) + sv;

      const Direction d = forward + left * (1 - 2 * u) + up * (1 - 2 * v);

      return Ray(eye + d, forward.normalize());
    }
};


#endif // CAMERA_H_
