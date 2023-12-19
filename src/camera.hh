#ifndef CAMERA_H_
#define CAMERA_H_

#include "film.hh"
#include "geometry.hh"
#include "ver.hh"

class Camera {
  public:

    Camera(size_t width, size_t height, size_t color_res = 10000)
      : film(width, height, color_res),
        nFilm(width, height, color_res),
        dFilm(width, height, color_res) {}

    virtual Ray getRay(size_t x, size_t y) const = 0;
    virtual void writeColor(size_t x, size_t y, const Direction &color) = 0;
    virtual void writeNormal(size_t x, size_t y, const Direction &normal) = 0;
    virtual void writeDepth(size_t x, size_t y, Float depth) = 0;

  public: // Portected
    Film film;
    Film nFilm, dFilm; // normal and depth
};

class PinholeCamera : public Camera {
  public:
    PinholeCamera(size_t width, size_t height, const Point &eye_, const Direction &left_, const Direction &up_, const Direction &forward_)
      : Camera(width, height), eye(eye_), left(left_), up(up_), forward(forward_) {
        // TODO: Check orthogonality and normalization
      }

    Ray getRay(size_t x, size_t y) const override {
      assert(x <= film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      const Float u = ((film.getWidth()-1 - x) - (Float)film.getWidth() / 2.0f) / ((Float)film.getWidth() / 2.0f);
      const Float v = ((film.getHeight()-1 - y) - (Float)film.getHeight() / 2.0f) / ((Float)film.getHeight() / 2.0f);


      const Float du = ((film.getWidth()-1 - 1) - (Float)film.getWidth() / 2.0f) / ((Float)film.getWidth() / 2.0f) - ((film.getWidth()-1) - (Float)film.getWidth() / 2.0f) / ((Float)film.getWidth() / 2.0f);
      const Float dv = ((film.getHeight()-1 - 1) - (Float)film.getHeight() / 2.0f) / ((Float)film.getHeight() / 2.0f) - ((film.getHeight()-1) - (Float)film.getHeight() / 2.0f) / ((Float)film.getHeight() / 2.0f);

      const Float su = uniform(0.0f, du);
      const Float sv = uniform(0.0f, dv);

      const Direction d = forward + up * (v+sv) + left * (u+su);

      return Ray(eye, d.normalize());
    }

    void writeColor(size_t x, size_t y, const Direction &color) override {
      assert(x < film.getWidth(), "x < width");
      assert(y < film.getHeight(), "y < height");

      assert(color.x >= 0, "x >= 0");
      assert(color.y >= 0, "y >= 0");
      assert(color.z >= 0, "z >= 0");

      Pixel &px = film[y * film.getHeight() + x];

      px.r += color.x;
      px.g += color.y;
      px.b += color.z;
    }

    void writeNormal(size_t x, size_t y, const Direction &normal) override {
      assert(x < film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      Pixel &px = nFilm[y * film.getHeight() + x];

      px.r = std::abs(normal.x);
      px.g = std::abs(normal.y);
      px.b = std::abs(normal.z);
    }

    void writeDepth(size_t x, size_t y, Float depth) override {
      assert(x < film.getWidth(), "x < width");
      assert(y <= film.getHeight(), "y < height");

      assert(depth > 0, "depth > 0");

      Pixel &px = dFilm[y * film.getHeight() + x];

      px.r = depth;
      px.g = depth;
      px.b = depth;
    }

  private:
    Point eye;
    Direction left, up, forward;
};


#endif // CAMERA_H_
