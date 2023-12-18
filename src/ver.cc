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

#define COLOR_SCHEMA 0
#define BALLS 1

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

  auto LBMaterial = std::make_shared<Slides::Material>(light_blue/1.5, Direction(1,1,1) - light_blue/1.5, black, black);
  // auto LBMaterial = std::make_shared<Material>(black, Direction(1,1,1), black, black);
  // auto LBMaterial = std::make_shared<Material>(pink, black, black, black);
  
  auto RBMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);
  // auto RBMaterial = std::make_shared<Slides::Material>(light_blue, black, black, black);

  #if COLOR_SCHEMA == 0
  scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;
  #elif COLOR_SCHEMA == 1
  scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = whiteMaterial;
  auto rightMaterial = whiteMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = redMaterial;
  auto botMaterial = greenMaterial;
  #elif COLOR_SCHEMA == 2
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterialE;
  auto botMaterial = whiteMaterial;

  LBMaterial = std::make_shared<Slides::Material>(pink, black, black, black);
  RBMaterial = std::make_shared<Slides::Material>(light_blue, black, black, black);
  #endif

  //===============================================================
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

  // FIXME: TODO: FOR WHATEVER REASON TOP AND BOTTOM NORMALS DONT WORK WITH THEIR CORRECT VALUES BUT WITH THE OTHERS WHAT?
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

  //===============================================================

  // auto meshFront = Plane(Point(0, 0, -5), Direction(0, 1, 0), Direction(1, 0, 0));
  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Triangle>(meshFront, 0),
  //             whiteMaterial));
  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Triangle>(meshFront, 1),
  //             whiteMaterial));

  #if BALLS
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3),
              LBMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3),
              RBMaterial));
  #endif

  
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

int main() {
  #if 1
  Scene cornellBoxScene;
  CornellBox(cornellBoxScene);

  //Scene mirrorScene;
  //MirrorScene(mirrorScene);

  int width = 1280/8;//720;
  int height = 1280/8;

  #if 1
  Point O(0, 0, -3.5);
  Direction left(-1, 0, 0), up(0, 1, 0), forward(0, 0, 3);
  PinholeCamera cam(width, height, O, left, up, forward);
  #else
  Point O(0.5, 1, -0.25);
  Direction left(0, 0, 1), up(-1, 0, 0), forward(0, -9.5, 0);
  PinholeCamera cam(width, height, O, left, up, forward);
  #endif
  // Point O(0, -0.5, 0);
  // Direction left(0, 0, -1), up(0, 1, 0), forward(-0.7, -0.1, 0);
  // PinholeCamera cam(width, height, O, left, up, forward.normalize());

  //raytracer::render(cam, cornellBoxScene, 6);
  //raytracer::render(cam, mirrorScene, 6);

  size_t spp = 128 * 2;
  size_t maxDepth = 1024;
  HemisphereSampler sampler = COSINE;
  pathtracer::render(cam, cornellBoxScene, spp, maxDepth, sampler);
  // photonmapper::render(cam, cornellBoxScene, 101);


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

  //ply::read("../model.ply");
  #endif

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
