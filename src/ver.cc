#include "ver.hh"
#include "geometry.hh"

#include "image/io.hh"
#include "image/film.hh"
#include "image/tonemap.hh"

#include "integrators/pathtracer.hh"
#include "integrators/photonmapper.hh"

#include "utils/argparse.hh"

#include "scenes.hh"

using namespace utils;

#ifdef VIEWER
#include "viewer.hh"
#endif

int ver(int argc, char **argv); // Main program
void merge(const std::unordered_map<std::string, std::vector<std::string>> &args);

int main(int argc, char **argv) {
  #ifndef VIEWER
    return ver(argc, argv);
  #else
    (void)argc;(void)argv;
    int width = 800/4;
    int height = 500/4;
    // Scene scene = CornellBox(width, height); 
    // Scene scene = Cardioid(width, height);
    Scene scene = Bunny(width, height);
    scene.makeBVH();

    Viewer viewer(scene);

    viewer.run();
  #endif
}

int ver(int argc, char **argv) {
  ArgumentParser parser("ver", "A simple pathtracer / photonmapper from scratch (with tonemappers)");

  parser.addArgument("integrator", "Integrator to use")
    .choices({"pathtracer", "photonmapper"})
    .default_value("pathtracer");
    // .default_value("photonmapper");

  parser.addArgument("--width", "Image width")
    .default_value("256");
  
  parser.addArgument("--height", "Image height")
    .default_value("256");

  parser.addArgument("--spp", "Samples per pixel")
    .default_value("64");
  
  parser.addArgument("-d", "Max recursion depth")
    .default_value("42");

  parser.addArgument("--normals", "Save scene normals image")
    .default_value("false")
    .flag();
  
  parser.addArgument("--depth", "Save depth image")
    .default_value("false")
    .flag();
  
  parser.addArgument("-t", "Tonemap to use")
    .choices({"gamma", "reinhard2002", "reinhard2005"})
    .default_value("gamma");
  
  parser.addArgument("-g", "Gamma value")
    .default_value("2.2");
  
  parser.addArgument("--sampler", "Hemisphere sampling method")
    .choices({"solid_angle", "cosine"})
    .default_value("cosine");
  
  parser.addArgument("-f", "Filename to save the image")
    .default_value("test.ppm");
  
  parser.addArgument("--hdr", "Save as HDR")
    .default_value("false")
    .flag();
  
  parser.addArgument("--bvh", "Use BVH")
    .default_value("true");
  
  parser.addArgument("--merge", "Merge HDR files into a single one and exit")
    .nargs('*');


  auto args = parser.parse(argc, argv);

  const auto &merge_files = args["--merge"];

  if (merge_files.size() > 0) {
    merge(args);
    exit(0);
  }

  #if 0
  // Print args
  std::cout << "Arguments:" << std::endl;
  for (const auto &arg : args) {
    std::cout << "  " << arg.first << ": ";
    for (const auto &value : arg.second)
      std::cout << value << " ";
    std::cout << std::endl;
  }
  exit(0);
  #endif

  // TODO: constify
  std::string integrator = args["integrator"][0];
  int width = std::stoi(args["--width"][0]);
  int height = std::stoi(args["--height"][0]);
  size_t spp = std::stoi(args["--spp"][0]);
  size_t maxDepth = std::stoi(args["-d"][0]);
  bool saveNormals = args["--normals"][0] == "true";
  bool saveDepth = args["--depth"][0] == "true";
  std::string tonemap = args["-t"][0];
  Float gamma = std::stof(args["-g"][0]);
  HemisphereSampler sampler = (args["--sampler"][0] == "solid_angle") ? SOLID_ANGLE : COSINE;
  std::string filename = args["-f"][0];
  bool saveHDR = args["--hdr"][0] == "true";
  bool useBVH = args["--bvh"][0] == "true";

  // Scene scene = CornellBox(width, height);
  // Scene scene = TriangleTextureTest(width, height);
  // Scene scene = SphereTextureTest(width, height);
  // Scene scene = Bunny(width, height);
  Scene scene = Cardioid(width, height);
  // Scene scene = Cardioid2(width, height);
  // Scene scene = LTO(width, height);
  // width = height = 512;

  if (useBVH)
    scene.makeBVH();

  // if (camera == "orthographic")
  //   cam = OrthographicCamera(width, height, O, left, up, forward);

  if (integrator == "pathtracer")
    pathtracer::render(scene.camera, scene, spp, maxDepth, sampler);
  else if (integrator == "photonmapper")
    photonmapper::render(scene.camera, scene, spp, maxDepth, sampler);

  auto &colorFilm = scene.camera->film;
  auto &normalFilm = scene.camera->nFilm;
  auto &depthFilm = scene.camera->dFilm;
  
  if (saveHDR) {
    image::write("hdr_" + filename, colorFilm);
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
    image::tonemap::Reinhard2002().applyTo(depthFilm);
    image::write("depth_" + filename, depthFilm);
  }

  return 0;
}

// TODO: TEST
void merge(const std::unordered_map<std::string, std::vector<std::string>> &args) {
  const auto &files = args.at("--merge");
  const std::string filename = args.at("-f")[0];
  const std::string tonemap = args.at("-t")[0];
  const Float gamma = std::stof(args.at("-g")[0]);

  image::Film out = image::read(files[0]);
  const size_t width = out.getWidth();
  const size_t height = out.getHeight();
  const size_t colorRes = out.getColorRes();

  for (size_t i = 1; i < files.size(); i++) {
    image::Film file = image::read(files[0]);
    const size_t w = file.getWidth();
    const size_t h = file.getHeight();
    const size_t c = file.getColorRes();

    if (w != width || h != height || c != colorRes)
      throw std::runtime_error("Images must have the same dimensions and color resolution");

    out.buffer += file.buffer;
  }

  out.buffer /= static_cast<Float>(files.size());

  if (tonemap == "gamma")
    image::tonemap::Gamma(gamma, out.max()).applyTo(out);
  else if (tonemap == "reinhard2002")
    image::tonemap::Reinhard2002().applyTo(out);
  else if (tonemap == "reinhard2005")
    image::tonemap::Reinhard2005().applyTo(out);
  else
    throw std::runtime_error("Unknown tonemap");
}
