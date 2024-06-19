#include "ver.hh"
#include "geometry.hh"
#include "image/io.hh"
#include "image/film.hh"
#include "image/tonemap.hh"
#include "integrators/pathtracer.hh"
#include "integrators/photonmapper.hh"
#include "utils/argparse.hh"
#include <chrono>

using namespace utils;

#ifdef VIEWER
#include "viewer.hh"
#else
#include "scenes.hh"

int ver(int argc, char **argv); // Main program
void merge(const std::unordered_map<std::string, std::vector<std::string>> &args);
#endif

int main(int argc, char **argv) {
  #ifndef VIEWER
    return ver(argc, argv);
  #else
    (void)argc;(void)argv;
    Viewer viewer(500, 500);
    viewer.run();
  #endif
}

#ifndef VIEWER
int ver(int argc, char **argv) {
  ArgumentParser parser("ver", "A simple pathtracer / photonmapper from scratch (with tonemappers)");

  parser.addArgument("integrator", "Integrator to use")
    .choices({"pathtracer", "photonmapper"})
    .default_value("pathtracer");

  parser.addArgument("--scene", "Scene to render")
    .choices({"cornellbox", "cornellboxA", "bunny", "orb", "nephroid",
              "cornellboxDiffuse", "cornellboxDiffuseA",
              "cornellboxTeapot", "cornellboxLucy", "cornellboxLucyA",
              "cornellboxMonkey", "cornellboxEye"})
    .default_value("cornellbox");

  parser.addArgument("--width", "Image width")
    .default_value("256");
  
  parser.addArgument("--height", "Image height")
    .default_value("256");

  parser.addArgument("--spp", "Samples per pixel")
    .default_value("64");

  parser.addArgument("--camera", "Camera to use")
    .choices({"pinhole", "orthographic"})
    .default_value("pinhole");

  parser.addArgument("-o", "Filename to save the image")
    .default_value("a.ppm");
  
  parser.addArgument("-d", "Max recursion depth")
    .default_value("42");
  
  parser.addArgument("--photons", "Number of photons to shoot (PhotonMapper)")
    .default_value("1000000");

  parser.addArgument("--k", "Number of neighbors (PhotonMapper)")
    .default_value("10000");

  parser.addArgument("--radius", "Radius for photon search (PhotonMapper)")
    .default_value("0.1");

  parser.addArgument("--nee", "Use next event estimation (PhotonMapper)")
    .default_value("false")
    .flag();

  parser.addArgument("--normals", "Save scene normals image")
    .default_value("false")
    .flag();
  
  parser.addArgument("--depth", "Save depth image")
    .default_value("false")
    .flag();
  
  parser.addArgument("-t", "Tonemap to use")
    .choices({"gamma", "reinhard2002"})
    .default_value("gamma");
  
  parser.addArgument("-g", "Gamma value")
    .default_value("2.2");
  
  parser.addArgument("--sampler", "Hemisphere sampling method")
    .choices({"solid_angle", "cosine"})
    .default_value("cosine");
  
  parser.addArgument("--hdr", "Save as HDR")
    .default_value("false")
    .flag();
  
  parser.addArgument("--bvh", "Use BVH")
    .default_value("true");
  
  parser.addArgument("--merge", "Merge HDR files into a single one and exit")
    .nargs('*');

  // TODO? Camera parameters?

  auto args = parser.parse(argc, argv);

  const auto &merge_files = args["--merge"];

  if (merge_files.size() > 0) {
    merge(args);
    exit(0);
  }

  const std::string &integrator = args["integrator"][0];
  const std::string &scn = args["--scene"][0];
  const int width = std::stoi(args["--width"][0]);
  const int height = std::stoi(args["--height"][0]);
  const size_t spp = std::stoi(args["--spp"][0]);
  const std::string &camera = args["--camera"][0];
  const std::string &filename = args["-o"][0];
  const size_t maxDepth = std::stoi(args["-d"][0]);
  const bool saveNormals = args["--normals"][0] == "true";
  const bool saveDepth = args["--depth"][0] == "true";
  const std::string &tonemap = args["-t"][0];
  const Float gamma = std::stof(args["-g"][0]);
  const HemisphereSampler sampler = (args["--sampler"][0] == "solid_angle") ? SOLID_ANGLE : COSINE;
  const bool saveHDR = args["--hdr"][0] == "true";
  const bool useBVH = args["--bvh"][0] == "true";
  // Args for photonmapper
  const size_t N = std::stoi(args["--photons"][0]);
  const size_t k = std::stoi(args["--k"][0]);
  const Float radius = std::stof(args["--radius"][0]);
  const bool nee = args["--nee"][0] == "true";

  // Scenes
  Scene scene;
  std::cout << "Loading scene..." << std::endl;
  if (scn == "bunny")
    scene = Bunny(width, height, camera);
  else if (scn == "orb")
    scene = LTO(width, height, camera);
  else if (scn == "nephroid")
    scene = Cardioid(width, height, camera);
  else if (scn == "cornellboxDiffuse")
    scene = CornellBox(width, height, camera, 0);
  else if (scn == "cornellboxDiffuseA")
    scene = CornellBox(width, height, camera, 1);
  else if (scn == "cornellboxTeapot")
    scene = CornellBox(width, height, camera, 2);
  else if (scn == "cornellboxLucy")
    scene = CornellBox(width, height, camera, 3);
  else if (scn == "cornellboxLucyA")
    scene = CornellBox(width, height, camera, 4);
  else if (scn == "cornellbox")
    scene = CornellBox(width, height, camera, 5);
  else if (scn == "cornellboxA")
    scene = CornellBox(width, height, camera, 6);
  else if (scn == "cornellboxMonkey")
    scene = CornellBox(width, height, camera, 7);
  else if (scn == "cornellboxEye")
    scene = CornellBoxR(width, height, camera);
  else
    throw std::runtime_error("(this should not happen) Unknown scene: " + scn);

  if (useBVH) {
    std::cout << "Building BVH..." << std::endl;
    scene.makeBVH();
  }

  // Seed
  #ifdef NDEBUG
  uint seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  #else
  uint seed = 5489u;
  #endif

  std::cout << "N: " << N << ", k: " << k << ", radius: " << radius << ", nee" << nee << std::endl;

  // Render
  if (integrator == "pathtracer")
    pathtracer::render(scene.camera, scene, spp, maxDepth, sampler, seed);
  else if (integrator == "photonmapper")
    photonmapper::render(scene.camera, scene, spp, maxDepth, N, k, radius, nee, sampler); // TODO: args

  auto &colorFilm = scene.camera->film;
  auto &normalFilm = scene.camera->nFilm;
  auto &depthFilm = scene.camera->dFilm;
  
  if (saveHDR) {
    image::write(filename + ".hdr", colorFilm);
  } else {
    if (tonemap == "gamma")
      image::tonemap::Gamma(gamma, colorFilm.max()).applyTo(colorFilm);
    else if (tonemap == "reinhard2002")
      image::tonemap::Reinhard2002().applyTo(colorFilm);
    else if (tonemap == "reinhard2005")
      image::tonemap::Reinhard2005().applyTo(colorFilm);
    else
      throw std::runtime_error("Unknown tonemap");

    image::write(filename, colorFilm);
  }

  if (saveNormals) {
    image::tonemap::Reinhard2002().applyTo(normalFilm);
    image::write("normals_" + filename, normalFilm);
  }

  if (saveDepth) {
    // image::tonemap::Reinhard2002().applyTo(depthFilm);
    image::tonemap::Gamma(1/2.2, depthFilm.max()).applyTo(depthFilm);
    image::write("depth_" + filename, depthFilm);
  }

  return 0;
}

void merge(const std::unordered_map<std::string, std::vector<std::string>> &args) {
  const auto &files = args.at("--merge");
  const std::string filename = args.at("-o")[0];
  const std::string tonemap = args.at("-t")[0];
  const Float gamma = std::stof(args.at("-g")[0]);
  const Float k = static_cast<Float>(files.size());

  image::Film out = image::read(files[0]);
  const size_t width = out.getWidth();
  const size_t height = out.getHeight();
  const size_t colorRes = out.getColorRes();

  std::cout << "Merging " << files[0] << std::endl;
  for (size_t i = 1; i < files.size(); i++) {
    std::cout << "Merging " << files[i] << std::endl;
    const image::Film file = image::read(files[i]);
    const size_t w = file.getWidth();
    const size_t h = file.getHeight();
    const size_t c = file.getColorRes();

    if (w != width || h != height || c != colorRes)
      throw std::runtime_error("Images must have the same dimensions and color resolution");

    out.buffer += file.buffer;
  }

  out.buffer /= k;

  if (tonemap == "gamma")
    image::tonemap::Gamma(gamma, out.max()).applyTo(out);
  else if (tonemap == "reinhard2002")
    image::tonemap::Reinhard2002().applyTo(out);
  else if (tonemap == "reinhard2005")
    image::tonemap::Reinhard2005().applyTo(out);
  else
    throw std::runtime_error("Unknown tonemap");
  
  image::write(filename, out);
}
#endif