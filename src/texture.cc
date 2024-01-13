#include "texture.hh"
#include "image/io.hh"
#include "utils/random.hh"

ConstantTexture::ConstantTexture(const Spectrum &k_) : k(k_) {}

Spectrum ConstantTexture::value(const SurfaceInteraction &/*interact*/) const { return k; }

PPMTexture::PPMTexture(const image::Framebuffer &fb_, Float su_, Float sv_) : fb(fb_), su{su_}, sv{sv_} {}

Spectrum PPMTexture::value(const SurfaceInteraction &interact) const {
  const Float u = interact.u;
  const Float v = interact.v;

  const size_t width = fb.getWidth() - 1;
  const size_t height = fb.getHeight() - 1;

  const size_t i = static_cast<size_t>(u * su * width);
  const size_t j = static_cast<size_t>(v * sv * height);

  const image::Pixel c = fb.get(i, j);

  return Spectrum(c.r, c.g, c.b);
}

Float PPMTexture::max() const {
  const size_t width = fb.getWidth();
  const size_t height = fb.getHeight();

  Float max = 0.0;

  for (size_t i = 0; i < width; i++)
    for (size_t j = 0; j < height; j++) {
      const image::Pixel c = fb.get(i, j);

      max = std::max(max, std::max(c.r, std::max(c.g, c.b)));
    }

  return max;
}

Float PPMTexture::min() const {
  const size_t width = fb.getWidth();
  const size_t height = fb.getHeight();

  Float min = std::numeric_limits<Float>::max();

  for (size_t i = 0; i < width; i++)
    for (size_t j = 0; j < height; j++) {
      const image::Pixel c = fb.get(i, j);

      min = std::min(min, std::min(c.r, std::min(c.g, c.b)));
    }

  return min;
}

Float turbulance(Float scale, Float roughness, Float u, Float v) {
  Float noise = 0.0;
  Float weight = 1.0;

  for (int i = 0; i < 7; i++) {
    noise += weight * utils::random::perlinNoise(scale * u, scale * v, 0.8);
    weight *= roughness;
    scale *= 2.0;
  }

  return std::abs(noise);
}


NoiseTexture::NoiseTexture(Float scale_, Float roughness_,
                           Float scale_u, Float scale_v,
                           const Spectrum &low_, const Spectrum &high_)
  : scale(scale_), roughness(roughness_), scaleU(scale_u), scaleV(scale_v),
    low(low_), high(high_) {}

Spectrum NoiseTexture::value(const SurfaceInteraction &interact) const {
  const Float u = interact.u * scaleU;
  const Float v = interact.v * scaleV;

  const Float noise = turbulance(scale, roughness, u, v);

  return low + (high - low) * noise;
}