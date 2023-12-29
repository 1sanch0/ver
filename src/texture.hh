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
    PPMTexture(const image::Framebuffer &fb);

    Spectrum value(const SurfaceInteraction &interact) const override;

  private:
    image::Framebuffer fb;
};

#endif // TEXTURE_H_
