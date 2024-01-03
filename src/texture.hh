#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "ver.hh"
#include "spectrum.hh"
#include "interaction.hh"
#include "image/framebuffer.hh"

class Texture {
  public:
    virtual Spectrum value(const SurfaceInteraction &interact) const = 0;
};

class ConstantTexture : public Texture {
  public:
    explicit ConstantTexture(const Spectrum &k_);

    Spectrum value(const SurfaceInteraction &interact) const override;

  private:
    Spectrum k;
};

class PPMTexture : public Texture {
  public:
    PPMTexture(const image::Framebuffer &fb_,
               Float su_ = 1.0, Float sv_ = 1.0);

    Spectrum value(const SurfaceInteraction &interact) const override;

  private:
    image::Framebuffer fb;
    Float su, sv;
};

class NoiseTexture : public Texture {
  public:
    NoiseTexture(Float scale_, Float roughness_,
                 Float scale_u = 1.0, Float scale_v = 1.0,
                 const Spectrum &low_ = Spectrum(0, 0, 0),
                 const Spectrum &high_ = Spectrum(1, 1, 1));

    Spectrum value(const SurfaceInteraction &interact) const override;

  private:
    Float scale, roughness;
    Float scaleU, scaleV;
    Spectrum low, high;
};

#endif // TEXTURE_H_
