#include "texture.hh"

ConstantTexture::ConstantTexture(const Spectrum &k_) : k(k_) {}

Spectrum ConstantTexture::value(const SurfaceInteraction &/*interact*/) const { return k; }

PPMTexture::PPMTexture(const image::Framebuffer &fb_) : fb(fb_) {}

Spectrum PPMTexture::value(const SurfaceInteraction &interact) const {
  const Float u = interact.u;
  const Float v = interact.v;

  const size_t width = fb.getWidth() - 1;
  const size_t height = fb.getHeight() - 1;

  const size_t i = static_cast<size_t>(u * width);
  const size_t j = static_cast<size_t>(v * height);

  const image::Pixel c = fb.get(i, j);

  return Spectrum(c.r, c.g, c.b);
}