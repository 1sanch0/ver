#ifndef SCENES_H_
#define SCENES_H_

#include "ver.hh"
#include "geometry.hh"

#include "shapes/sphere.hh"
#include "shapes/triangle.hh"
#include "shapes/quad.hh"

#include "utils/simply.hh"

#include "materials/slides.hh"
#include "materials/tex.hh"

#include "scene.hh"

#define WALLS_H 0
#define WALLS_V_BALLS 1
#define DEFAULT_PT 2
#define TEAPOT 3
#define DEFAULT_FINAL 4
#define SCULPTURE 5

#define VERSION 0

Scene CornellBox(size_t width, size_t height) {
  Scene scene;

  Point O(0.0, 0.0, -3.5);
  Direction left(-1, 0, 0), up(0, 1, 0), forward(0, 0, 3);
  const auto cam = std::make_shared<PinholeCamera>(width, height, O, left, up, forward);
  scene.set(cam);

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

  simply::PLYFile sus("../teapotN.ply");
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
  // auto RBMaterial = std::make_shared<Slides::Material>(black, Direction(1,1,1), black, black);

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
  #elif VERSION == 5
  scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto lucyMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);
  // auto lucyMaterial = std::make_shared<Slides::Material>(Direction(1,1,1), black, black, white);

  simply::PLYFile sus("../lucy.ply");
  auto meshSus = std::make_shared<TriangleMesh>(
    Mat4::translation(-0.5, -0.4, 0) * Mat4::scale(6, 6, 6),
    // Mat4::identity(),
    sus);

  std::cout << "TRI: " << meshSus->nTriangles << std::endl;
  for (size_t i = 0; i < meshSus->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshSus, i),
        lucyMaterial));

  #endif

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
  
  return scene;
}

Scene Bunny(bool pointLight = true, bool areaLight = true) {
  Scene scene;

  const auto none = Direction(0, 0, 0);
  const auto white = Direction(.9, .9, .9);
  const auto red   = Direction(.9, .2, .2);
  const auto green = Direction(.2, .9, .2);
  const auto pink = Direction(0.8941, 0.66667, 0.9);

  auto pinkMaterial = std::make_shared<Slides::Material>(pink, none, none, none);
  auto whiteMaterial = std::make_shared<Slides::Material>(white, none, none, none);
  auto redMaterial = std::make_shared<Slides::Material>(red, none, none, none);
  auto greenMaterial = std::make_shared<Slides::Material>(green, none, none, none);
  auto emmMaterial = std::make_shared<Slides::Material>(white, none, none, white);

  if (pointLight)
    scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));

  // auto bunnyMaterial = std::make_shared<Slides::Material>(none, white, none, none);
  auto bunnyMaterial = pinkMaterial;

  simply::PLYFile bunny("../bunny.ply");
  auto meshBunny = std::make_shared<TriangleMesh>(
    Mat4::translation(0, -0.6230, 0) * Mat4::scale(6, 6, 6),
    bunny);
  
  for (size_t i = 0; i < meshBunny->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshBunny, i),
        bunnyMaterial));
  

  auto meshLeft = Quad(Point(-1, -0.7, 0), Direction(0, 1, 0), Direction(0, 0, 1), Direction(1, 0, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshLeft, 0),
              greenMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshLeft, 1),
              greenMaterial));

  auto meshRight = Quad(Point(1, -0.7, 0), Direction(0, 1, 0), Direction(0, 0, 1), (Direction(-1, 0, 0)));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshRight, 0),
              redMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshRight, 1),
              redMaterial));

  auto meshBack = Quad(Point(0, 0, 1), Direction(0, 10, 0), Direction(10, 0, 0), Direction(0, 0, -1));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBack, 0),
              whiteMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBack, 1),
              whiteMaterial));

  auto meshBot = Quad(Point(0, -1, 0), Direction(-1, 0, 0), Direction(0, 0, -1), Direction(0, 1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBot, 0),
              whiteMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBot, 1),
              whiteMaterial));

  if (areaLight) {
    auto meshTop = Quad(Point(0, 0.4, 0), Direction(100, 0, 0), Direction(0, 0, 1), Direction(0, -1, 0));
    scene.add(std::make_unique<GeometricPrimitive>(
                std::make_shared<Triangle>(meshTop, 0),
                emmMaterial));
    scene.add(std::make_unique<GeometricPrimitive>(
                std::make_shared<Triangle>(meshTop, 1),
                emmMaterial));
  }
  
  return scene;
}

// Scene TextureTest() {
//   const auto none = Direction(0, 0, 0);
//   const auto white = Direction(.9, .9, .9);
//   const auto minecraftBlue = Spectrum(0.431, 0.694, 1.0);

//   const auto grayLow = Spectrum(0.467, 0.467, 0.467);
//   const auto grayHigh = Spectrum(0.667, 0.663, 0.678);

//   const auto env = image::read("../envmap.ppm");
//   // const auto eye = image::read("../eye.ppm");

//   const auto texNone = std::make_shared<ConstantTexture>(none);
//   const auto texWhite = std::make_shared<ConstantTexture>(white);
//   // const auto texTestUV = std::make_shared<PPMTexture>(env.buffer);
//   // const auto texEye = std::make_shared<PPMTexture>(eye.buffer);
//   const auto texTestUV = std::make_shared<NoiseTexture>(0.3, 0.9);
//   const auto texEye = std::make_shared<NoiseTexture>(1, 1, 1, 35, grayHigh, grayLow);
//   const auto texEyeReflect = std::make_shared<ConstantTexture>(Spectrum(0.2, 0.2, 0.2));

//   const auto botMaterial = std::make_shared<Slides::Material>(white, none, none, none);
//   // const auto ballMaterial = std::make_shared<Slides::Material>(Direction(1, 0, 0), none, none, none);
//   const auto ballMaterial = std::make_shared<tex::Material>(texTestUV, texNone, texNone);

//   const auto ballMaterial2 = std::make_shared<tex::Material>(texEye, texEyeReflect, texNone);

//   // const auto ballMaterial3 = std::make_shared<tex::Material>(texEye, texNone, texNone);

//   // Scene scene(std::make_unique<ConstantTexture>(minecraftBlue));
//   Scene scene(std::make_unique<PPMTexture>(env.buffer));

//   scene.add(LightPoint(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));

//   auto meshBot = Quad(Point(0, -1, 0), Direction(-10, 0, 0), Direction(0, 0, -10), Direction(0, 1, 0));
//   scene.add(std::make_unique<GeometricPrimitive>(
//               std::make_shared<Triangle>(meshBot, 0),
//               botMaterial));
//   scene.add(std::make_unique<GeometricPrimitive>(
//               std::make_shared<Triangle>(meshBot, 1),
//               botMaterial));

//   scene.add(std::make_unique<GeometricPrimitive>(
//             std::make_shared<Sphere>(Point(-0.3, -0.7, 0.0), 0.3),
//             ballMaterial));
//   scene.add(std::make_unique<GeometricPrimitive>(
//             std::make_shared<Sphere>(Point(0.3, -0.7, 0.0), 0.3),
//             ballMaterial2));

//   return scene;
// }

// Scene EnvMapTest(std::unique_ptr<Texture> env = nullptr) {
//   Scene scene(std::move(env));

//   const auto none = Direction(0, 0, 0);
//   const auto white = Direction(.9, .9, .9);

//   const auto refractMaterial = std::make_shared<Slides::Material>(none, none, white, none);
//   // const auto refractMaterial = std::make_shared<Slides::Material>(none, white, none, none);
//   // const auto refractMaterial = std::make_shared<Slides::Material>(white, none, none, none);

//   scene.add(std::make_unique<GeometricPrimitive>(
//             std::make_shared<Sphere>(Point(0.0, 0.0, 0.0), 0.3),
//             refractMaterial));

//   return scene;
// }

Scene LTO() {
  // const auto minecraftBlue = Spectrum(0.431, 0.694, 1.0);
  Scene scene;
  // Scene scene(std::make_unique<ConstantTexture>(minecraftBlue));

  const auto none = Direction(0, 0, 0);
  const auto white = Direction(.9, .9, .9);

  const auto grayLow = Spectrum(0.467, 0.467, 0.467);
  const auto grayHigh = Spectrum(0.667, 0.663, 0.678);

  const auto emitter = std::make_shared<Slides::Material>(none, none, none, white);
  const auto whiteMaterial = std::make_shared<Slides::Material>(white, none, none, none);
  const auto LTOMaterial = std::make_shared<Slides::Material>(none, none, white, none);

  auto meshEmitter = Quad(Point(0, 3.2, 0), Direction(-1, 0, 0), Direction(0, 0, -1), Direction(0, 1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshEmitter, 0),
              emitter));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshEmitter, 1),
              emitter));

  auto meshFloor = Quad(Point(0, 0, 0), Direction(-1, 0, 0), Direction(0, 0, -1), Direction(0, 1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshFloor, 0),
              whiteMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshFloor, 1),
              whiteMaterial));
  

  simply::PLYFile LTO("../../LTO.ply");
  auto meshLTO = std::make_shared<TriangleMesh>(
    Mat4::identity(),
    // Mat4::scale(6, 6, 6),
    LTO);
  
  for (size_t i = 0; i < meshLTO->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshLTO, i),
        LTOMaterial));

  return scene;
}

#endif // SCENES_H_