#include "ver.hh"
#include "geometry.hh"
#include "io/io.hh"
#include "film.hh"
#include "tonemap/tonemap.hh"
#include "shape.hh"

#include "io/simply.hh"

#include <algorithm>
#include <vector>
#include <initializer_list>

#include <chrono>

#include "camera.hh"
#include "material.hh"
#include "primitive.hh"
#include "lights.hh"

#include "accelerators/bvh.hh"
#include "scene.hh"

// #include "algorithms/raytracer.hh"
#include "algorithms/pathtracer.hh"
#include "algorithms/photonmapper.hh"

#include "utils/argparse.hh"

#define WALLS_H 0
#define WALLS_V_BALLS 1
#define DEFAULT_PT 2
#define TEAPOT 3
#define DEFAULT_FINAL 4


#define VERSION TEAPOT
#define USE_BVH 1
// #define DEBUG_BVH 0 && USE_BVH

void CornellBox(Scene &scene) {
  // scene.add(LightPoint(Point(0, -0.5, -1), Direction(1, 1, 1)));

  auto white = Direction(.9, .9, .9);
  auto whiteL= Direction(1, 1, 1);
  auto red   = Direction(.9, 0, 0);
  auto green = Direction(0, .9, 0);
  auto black = Direction(0, 0, 0);
  auto pink = Direction(0.8941, 0.66667, 0.9);
  auto light_blue = Direction(0.5529, 1, 1);

  auto whiteMaterial = std::make_shared<Slides::Material>(white, black, black, black);
  auto whiteMaterialE = std::make_shared<Slides::Material>(white, black, black, whiteL);
  auto whiteMaterialReflect = std::make_shared<Slides::Material>(black, white, black, black);
  auto redMaterial = std::make_shared<Slides::Material>(red, black, black, black);
  auto greenMaterial = std::make_shared<Slides::Material>(green, black, black, black);

  auto pinkMaterial = std::make_shared<Slides::Material>(pink, black, black, black);

  #if VERSION == 0
  scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto LBMaterial = std::make_shared<Slides::Material>(light_blue/1.5, Direction(1,1,1) - light_blue/1.5, black, black);
  auto RBMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);
  #elif VERSION == 1
  scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = whiteMaterial;
  auto rightMaterial = whiteMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = redMaterial;
  auto botMaterial = greenMaterial;

  auto LBMaterial = std::make_shared<Slides::Material>(light_blue/1.5, Direction(1,1,1) - light_blue/1.5, black, black);
  auto RBMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);

  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3),
              LBMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3),
              RBMaterial));
  #elif VERSION == 2
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterialE;
  auto botMaterial = whiteMaterial;

  auto LBMaterial = std::make_shared<Slides::Material>(pink, black, black, black);
  auto RBMaterial = std::make_shared<Slides::Material>(light_blue, black, black, black);

  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3),
              LBMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3),
              RBMaterial));
  #elif VERSION == 3
  scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto teapotMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);

  simply::PLYFile sus("../../teapotN.ply");
  auto meshSus = std::make_shared<TriangleMesh>(
    Mat4::rotate(M_PI / -2.0, 1, 0, 0) * Mat4::translation(0, 0, -1) * Mat4::scale(.2, .2, .2),
    sus);


  for (size_t i = 0; i < meshSus->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshSus, i),
        teapotMaterial));

  #elif VERSION == 4 
  // Focus
  scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto LBMaterial = std::make_shared<Slides::Material>(light_blue/1.5, Direction(1,1,1) - light_blue/1.5, black, black);
  auto RBMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);

  // auto CBMaterial = std::make_shared<Slides::Material>(light_blue, black, black, black);
  // auto LBMaterial = std::make_shared<Slides::Material>(pink, black, black, black);
  // auto RBMaterial = std::make_shared<Slides::Material>(black, black, white, black);

  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3),
              LBMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3),
              RBMaterial));

  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Sphere>(Point(0.0, 0.0, -0.8), 0.2),
  //             CBMaterial));
  #endif

  #if !DEBUG_BVH
  auto meshLeft = Quad(Point(-1, 0, 0), Direction(0, 1, 0), Direction(0, 0, 1), Direction(1, 0, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshLeft, 0),
              leftMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshLeft, 1),
              leftMaterial));

  auto meshRight = Quad(Point(1, 0, 0), Direction(0, 1, 0), Direction(0, 0, 1), (Direction(-1, 0, 0)));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshRight, 0),
              rightMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshRight, 1),
              rightMaterial));

  auto meshBack = Quad(Point(0, 0, 1), Direction(0, 1, 0), Direction(1, 0, 0), Direction(0, 0, -1));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBack, 0),
              backMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBack, 1),
              backMaterial));

  auto meshTop = Quad(Point(0, 1, 0), Direction(1, 0, 0), Direction(0, 0, 1), Direction(0, -1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshTop, 0),
              topMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshTop, 1),
              topMaterial));

  auto meshBot = Quad(Point(0, -1, 0), Direction(-1, 0, 0), Direction(0, 0, -1), Direction(0, 1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBot, 0),
              botMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBot, 1),
              botMaterial));

  #endif 

  #if USE_BVH
  // std::cout << "S': " << scene.scene.size() << std::endl;
  // std::vector<std::shared_ptr<Primitive>> p(scene.scene.size());
  // for (int i = 0; i < scene.scene.size(); i++)
  //   p[i] = std::move(scene.scene[i]);
  // p.reserve(p.size());
  // scene.scene = std::vector<std::unique_ptr<Primitive>>();
  // scene.scene.push_back(std::make_unique<BVH>(std::move(p)));
  #endif
  // std::cout << "S: " << scene.scene.size() << std::endl;

  // auto meshFront = Plane(Point(0, 0, -5), Direction(0, 1, 0), Direction(1, 0, 0));
  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Triangle>(meshFront, 0),
  //             whiteMaterial));
  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Triangle>(meshFront, 1),
  //             whiteMaterial));

  
  // simply::PLYFile sus("../teapot.ply");
  // auto meshSus = std::make_shared<TriangleMesh>(
  //   Mat4::rotate(M_PI * 1.0/4.0, 1, 0, 0) * Mat4::scale(.2, .2, .2),
  //   sus);

  // for (size_t i = 0; i < meshSus->nTriangles; i++)
  //   scene.add(
  //     std::make_unique<GeometricPrimitive>(
  //       std::make_shared<Triangle>(meshSus, i),
  //       pinkMaterial));



  // std::cout << "S: " << scene.scene.size() << std::endl;
  // std::vector<std::shared_ptr<Primitive>> p(scene.scene.size());
  // for (int i = 0; i < scene.scene.size(); i++)
  //   p[i] = std::move(scene.scene[i]);
  // p.reserve(p.size());
  // scene.scene = std::vector<std::unique_ptr<Primitive>>();
  // std::cout << "S: " << p.size() << std::endl;
  // scene.scene.push_back(std::make_unique<BVH>(std::move(p)));






  // TODO: ESTA MIERDA NO FUNCIONA
  //simply::PLYFile camera("../camera.ply");
  //auto meshCam = std::make_shared<TriangleMesh>(
    //Mat4::translation(-2, 0, 0) *
    //Mat4::scale(0.2, 0.2, 0.2) *
    ////Mat4::rotate(M_PI / 2, 1, 0, 0) *
    //Mat4::identity(),
    //camera);

  //for (size_t i = 0; i < meshCam->nTriangles; i++)
  //scene.add(std::make_unique<GeometricPrimitive>(
              //std::make_shared<Triangle>(meshCam, i),
              //redMaterial));

  // std::cout << "Scene with " << scene.scene.size() << " polygons!" << std::endl;
}

int main(int argc, char **argv) {
  utils::ArgumentParser parser("ver", "A simple pathtracer / photonmapper from scratch");

  parser.addArgument("integrator", "Integrator to use")
    .choices({"pathtracer", "photonmapper"})
    .default_value("pathtracer");

  parser.addArgument("--width", "Image width")
    .default_value("1280");
  
  parser.addArgument("--height", "Image height")
    .default_value("720");

  parser.addArgument("--spp", "Samples per pixel")
    .default_value("256");
  
  parser.addArgument("-d", "Max recursion depth")
    .default_value("24");

  parser.addArgument("--normals", "Save scene normals image")
    .default_value("false")
    .flag();
  
  parser.addArgument("--depth", "Save depth image")
    .default_value("false")
    .flag();
  
  parser.addArgument("-t", "Tonemap to use")
    .choices({"gamma", "reinhard2002", "reinhard2005"})
    .default_value("gamma");
  
  /*
  TODO: tonemap parameters
  
  parser.addArgument("-g", "Gamma value")
    .default_value("2.2");
  */
  
  parser.addArgument("-c", "Camera to use")
    .choices({"pinhole", "orthographic"})
    .default_value("pinhole");

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
    // Merge HDR files to a single one and exit

    // TODO: should be fun tho :)

    exit(0);
  }

  // TODO: perezaaaaa

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

  std::string integrator = args["integrator"][0];
  int width = std::stoi(args["--width"][0]);
  int height = std::stoi(args["--width"][0]);
  size_t spp = std::stoi(args["--spp"][0]);
  size_t maxDepth = std::stoi(args["-d"][0]);
  bool saveNormals = args["--normals"][0] == "true";
  bool saveDepth = args["--depth"][0] == "true";
  std::string tonemap = args["-t"][0];
  std::string camera = args["-c"][0];
  HemisphereSampler sampler = (args["--sampler"][0] == "solid_angle") ? SOLID_ANGLE : COSINE;
  std::string filename = args["-f"][0];
  bool saveHDR = args["--hdr"][0] == "true";
  bool useBVH = args["--bvh"][0] == "true";

  Scene cornellBoxScene;
  CornellBox(cornellBoxScene);
  if (useBVH)
    cornellBoxScene.makeBVH();

  Point O(0, 0, -3.5);
  Direction left(-1, 0, 0), up(0, 1, 0), forward(0, 0, 3);
  PinholeCamera cam(width, height, O, left, up, forward);

  // if (camera == "orthographic")
  //   cam = OrthographicCamera(width, height, O, left, up, forward);

  if (integrator == "pathtracer")
    pathtracer::render(cam, cornellBoxScene, spp, maxDepth, sampler);
  else if (integrator == "photonmapper")
    photonmapper::render(cam, cornellBoxScene, 101/*TODO*/);
  
  if (saveHDR) {
    image::write("hdr_" + filename, cam.film);
  } else {
    if (tonemap == "gamma")
      tonemap::Gamma(2.2, cam.film.max()).applyTo(cam.film);
    else if (tonemap == "reinhard2002")
      tonemap::Reinhard2002().applyTo(cam.film);
    else if (tonemap == "reinhard2005")
      tonemap::Reinhard2005().applyTo(cam.film);
    else
      assert(false, "Unknown tonemap");
  }

  if (saveNormals) {
    tonemap::Reinhard2002().applyTo(cam.nFilm);
    image::write("normals_" + filename, cam.nFilm);
  }

  if (saveDepth) {
    // tonemap::Reinhard2002().applyTo(cam.nFilm);
    // image::write("normals_" + filename, cam.nFilm);
  }

  tonemap::Gamma(2.2, cam.film.max()).applyTo(cam.film);
  // tonemap::Reinhard2002().applyTo(cam.film);
  std::cout << "Max: " << cam.film.max() << std::endl;
  // assert(cam.film.max() == 1, "fiml max !=1");
  image::write("test.ppm", cam.film);

  std::cout << "Max: " << cam.nFilm.max() << std::endl;
  tonemap::Reinhard2002().applyTo(cam.nFilm);
  // std::cout << "Max: " << filmNormal.max() << std::endl;
  assert(cam.nFilm.max() == 1, "film max !=1");
  image::write("testN.ppm", cam.nFilm);

  //tonemap::Reinhard2002().applyTo(cam.dFilm);
  //assert(cam.dFilm.max() == 1, "film max !=1");
  //image::write("testD.ppm", cam.dFilm);


  #if 0
  auto film = imageio::read("../HDR_PPM/seymour_park.ppm");
  Float max = film.max();

  auto copy1 = film;
  auto copy2 = film;
  auto copy3 = film;
  auto copy4 = film;

  //tonemap::Gamma(2.2, max).applyTo(film);
  //assert(film.max() == 1, "film max !=1");
  //imageio::write("gamma22eq.ppm", film);
//
  //tonemap::Gamma(1, max).applyTo(copy1);
  //assert(copy1.max() == 1, "copy1 max !=1");
  //imageio::write("eq.ppm", copy1);
//
  //tonemap::Gamma(1, 1).applyTo(copy2);
  //assert(copy2.max() == 1, "copy2 max !=1");
  //imageio::write("clamp.ppm", copy1);
//
  //tonemap::Reinhard2002().applyTo(copy3);
  //assert(copy3.max() == 1, "copy3 max !=1");
  //imageio::write("reinhard2002.ppm", copy3);

  tonemap::Reinhard2005().applyTo(copy4);
  assert(copy4.max() == 1, "copy4 max !=1");
  imageio::write("reinhard2005default.ppm", copy4);
  #endif


  return 0;
}
