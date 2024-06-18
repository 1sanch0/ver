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

#include "image/io.hh"

#include "scene.hh"

// The code below is so ass and messy bc i did it quickly
// beware

#define WALLS_H 0
#define WALLS_V_BALLS 1
#define DEFAULT_PT 2
#define TEAPOT 3
#define DEFAULT_FINAL 4
#define SCULPTURE 5
#define TEXTURES 6

#define VERSION DEFAULT_FINAL
// #define VERSION 5

Scene CornellBox(size_t width, size_t height) {
  Scene scene;

  Point O(0.0, 0.0, -3.5);
  Direction left(-1, 0, 0), up(0, 1, 0), forward(0, 0, 3);
  auto cam = std::make_shared<PinholeCamera>(width, height, O, left, up, forward);
  // auto cam = std::make_shared<OrthographicCamera>(width, height, O, left, up, forward);
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
  scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto LBMaterial = std::make_shared<Slides::Material>(pink, black, black, black);
  auto RBMaterial = std::make_shared<Slides::Material>(light_blue, black, black, black);

  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3),
              LBMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3),
              RBMaterial));
  #elif VERSION == 1
  scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
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

  scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  topMaterial = whiteMaterial;

  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3),
              LBMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3),
              RBMaterial));
  #elif VERSION == 3
  scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterialE;
  auto botMaterial = whiteMaterial;

  O = Point(0.8, 0.15, -2);
  Point lookAt(0, -0.5, 0);
  cam = std::make_shared<PinholeCamera>(width, height, O, lookAt, 3.0);
  scene.set(cam);

  Float alpha = 0.45;
  auto teapotMaterial = std::make_shared<Slides::Material>(light_blue * alpha, black, Direction(1,1,1)-light_blue * alpha, black);

  simply::PLYFile sus("assets/teapot.ply");
  auto meshSus = std::make_shared<TriangleMesh>(
    Mat4::rotate(M_PI / -2.0, 1, 0, 0) * Mat4::translation(0, 0, -0.9) * Mat4::scale(.2, .2, .2),
    sus);

  for (size_t i = 0; i < meshSus->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshSus, i),
        teapotMaterial));

  #elif VERSION == 4 
  bool pointLight = true;

  if (pointLight)
    scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));

  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto LBMaterial = std::make_shared<Slides::Material>(light_blue/1.5, Direction(1,1,1) - light_blue/1.5, black, black);
  auto RBMaterial = std::make_shared<Slides::Material>(black, Direction(1,1,1)*0.18, Direction(1,1,1)*0.82, black);
  // auto RBMaterial = std::make_shared<Slides::Material>(black, Direction(0.2,0.2,0.2), Direction(0.8,0.8,0.8), black);
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

  if (!pointLight) {
    auto meshAreaL = Quad(Point(0, 0.99, 0), Direction(0.5, 0, 0), Direction(0, 0, 0.5), Direction(0, -1, 0));
    scene.add(std::make_unique<GeometricPrimitive>(
                std::make_shared<Triangle>(meshAreaL, 0),
                whiteMaterialE));
    scene.add(std::make_unique<GeometricPrimitive>(
                std::make_shared<Triangle>(meshAreaL, 1),
                whiteMaterialE));
  }

  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Sphere>(Point(0.0, 0.0, -0.8), 0.2),
  //             CBMaterial));
  #elif VERSION == 5
  bool pointLight = false;

  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  // auto lucyMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);
  auto lucyMaterial = std::make_shared<Slides::Material>(Direction(0.9,0.9,0.9), black, black, black);

  simply::PLYFile sus("assets/lucy.ply");
  auto meshSus = std::make_shared<TriangleMesh>(
    Mat4::translation(-0.5, -0.4, -0.25) * Mat4::scale(6, 6, 6),
    // Mat4::identity(),
    sus);

  for (size_t i = 0; i < meshSus->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshSus, i),
        lucyMaterial));
  

  auto RBMaterial = std::make_shared<Slides::Material>(black, Direction(1,1,1), black, black);
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, 0.25), 0.3),
              RBMaterial));
  
  // scene.add(PointLight(Point(0, 0, -3.5), Direction(1., 1., 1.)));
  if (pointLight) {
    scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  } else {
    topMaterial = whiteMaterialE;
    // auto meshAreaL = Quad(Point(0, 0.99, 0), Direction(0.5, 0, 0), Direction(0, 0, 0.5), Direction(0, -1, 0));
    // scene.add(std::make_unique<GeometricPrimitive>(
    //             std::make_shared<Triangle>(meshAreaL, 0),
    //             whiteMaterialE));
    // scene.add(std::make_unique<GeometricPrimitive>(
    //             std::make_shared<Triangle>(meshAreaL, 1),
    //             whiteMaterialE));
  }

  #elif VERSION == 6
  scene.add(PointLight(Point(0, 0.5, 0), Direction(0.5, 0.5, 0.5)));
  // scene.add(PointLight(Point(0.5, 0, -1), Direction(0.1, 0.1, 0.1)));
  scene.set(std::make_shared<PPMTexture>(image::read("assets/envmap.ppm").buffer));

  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto RBMaterial = std::make_shared<Slides::Material>(black, Direction(1,1,1), black, black);
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3),
              RBMaterial));

  auto banana = image::read("assets/banana.ppm");
  auto harambe = image::read("assets/harambe.ppm");
  auto bananaTex = std::make_shared<PPMTexture>(banana.buffer);
  auto harambeTex = std::make_shared<PPMTexture>(harambe.buffer);
  auto blackTex = std::make_shared<ConstantTexture>(black);
  auto bananaMaterial = std::make_shared<tex::Material>(bananaTex, blackTex, blackTex);
  auto harambeMaterial = std::make_shared<tex::Material>(harambeTex, blackTex, blackTex);

  simply::PLYFile model("assets/banana.ply");
  auto mesh= std::make_shared<TriangleMesh>(
    Mat4::rotate(M_PI, 1, 0, 0) *
    Mat4::translation(-0.4, 0.9, 0) *
    Mat4::scale(0.2, 0.2, 0.2),
    model);

  for (size_t i = 0; i < mesh->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(mesh, i),
        bananaMaterial));
  
  const Float w = harambe.buffer.getWidth();
  const Float h = harambe.buffer.getHeight();
  const Float m = std::max(w, h);
  const Float alpha = 0.9;

  auto meshH = Quad(Point(0, 0, 0.99), Direction(-w/m, 0, 0)*alpha, Direction(0, -h/m, 0)*alpha, Direction(0, 0, -1));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshH, 0),
              harambeMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshH, 1),
              harambeMaterial));

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

Scene Cardioid(size_t width, size_t height) {
  // const auto env = image::read("../envmap.ppm");

  Scene scene;
  // scene.set(std::make_shared<PPMTexture>(env.buffer));

  Point O(0, 2.4, -0.1);
  Point lookAt(0, -1, 0);
  const auto cam = std::make_shared<PinholeCamera>(width, height, O, lookAt, 3.5);
  scene.set(cam);

  const auto none = Direction(0, 0, 0);
  const auto white = Direction(.9, .9, .9);

  auto whiteMaterial = std::make_shared<Slides::Material>(white, none, none, none);
  auto emmisiveMaterial = std::make_shared<Slides::Material>(none, none, none, white);
  auto mugMaterial = std::make_shared<Slides::Material>(Direction(0.70,0.70,0.70), Direction(0.3, 0.3, 0.3), none, none);

  const auto woodIm = image::read("assets/wood.ppm");
  const auto texWood = std::make_shared<PPMTexture>(woodIm.buffer);
  const auto texNone = std::make_shared<ConstantTexture>(none);

  const auto floorMaterial = std::make_shared<tex::Material>(texWood, texNone, texNone);

  simply::PLYFile mug("assets/mug.ply");
  auto mesh = std::make_shared<TriangleMesh>(
    // Mat4::rotate(-0.4, 1, 0, 0) *
    // Mat4::translation(0, -0.6, 0) *
    Mat4::translation(0, -0.65, 0) *
    Mat4::scale(4, 4, 4),
    mug);
  
  for (size_t i = 0; i < mesh->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(mesh, i),
        mugMaterial));


  // scene.add(PointLight(Point(0, 0.2, 0.6), Direction(0.2, 0.2, 0.2)));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0, 1, 1), 0.1),
              emmisiveMaterial));
  
  auto meshBot = Quad(Point(0, -1, 0), Direction(-2, 0, 0), Direction(0, 0, -2), Direction(0, 1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBot, 0),
              floorMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshBot, 1),
              floorMaterial));

  scene.add(PointLight(Point(0, 100, -100), Direction(30, 30, 30)));
  // auto meshTop = Quad(Point(0, 200, -300), Direction(-10, 0, 0), Direction(0, -10, 0), Direction(0, 1, 0));
  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Triangle>(meshTop, 0),
  //             emmisiveMaterial));
  // scene.add(std::make_unique<GeometricPrimitive>(
  //             std::make_shared<Triangle>(meshTop, 1),
  //             emmisiveMaterial));

  return scene;
}

Scene Cardioid2(size_t width, size_t height) {
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

  scene.add(PointLight(Point(0.3, 0.1, 0), Direction(0.1, 0.1, 0.1)));

  auto leftMaterial = redMaterial;
  auto rightMaterial = greenMaterial;
  auto backMaterial = whiteMaterial;
  auto topMaterial = whiteMaterial;
  auto botMaterial = whiteMaterial;

  auto LBMaterial = std::make_shared<Slides::Material>(light_blue/1.5, Direction(1,1,1) - light_blue/1.5, black, black);
  auto RBMaterial = std::make_shared<Slides::Material>(black, black, Direction(1,1,1), black);

  simply::PLYFile egg("../../egg.ply");
  auto meshEgg = std::make_shared<TriangleMesh>(
    Mat4::translation(-0.2, -0.6230, -0.1) * Mat4::scale(4, 4, 4),
    egg);
  
  for (size_t i = 0; i < meshEgg->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshEgg, i),
        RBMaterial));

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

Scene Bunny(size_t width, size_t height) {
  Scene scene;

  bool pointLight = false;
  bool areaLight = true;

  Point O(0, 0, -3.5);
  // Point lookAt(0, -1, 0);
  Point lookAt(0, -0.5, 0);
  const auto cam = std::make_shared<PinholeCamera>(width, height, O, lookAt, 4.1);
  scene.set(cam);

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
    scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));

  auto bunnyMaterial = std::make_shared<Slides::Material>(pink, white-pink *3/5, none, none);

  simply::PLYFile bunny("assets/bunny.ply");
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

Scene CornellBoxR(size_t width, size_t height) {
  Scene scene;

  Point O(-3.5, 0.0, 0);
  Point lookAt(0, 0, 0);
  // Direction left(1, 0, 0), up(0, 1, 0), forward(0, 0, 3);
  auto cam = std::make_shared<PinholeCamera>(width, height, O, lookAt, 3);
  scene.set(cam);

  scene.add(PointLight(Point(0, 0.5, 0), Direction(0.1, 0.1, 0.1)));
  scene.add(PointLight(Point(-0.5, 0.5, 1), Direction(0.1, 0.1, 0.1)));

  auto white = Direction(.9, .9, .9);
  auto red   = Direction(.9, 0, 0);
  auto green = Direction(0, .9, 0);
  auto black = Direction(0, 0, 0);

  auto frontMaterial = std::make_shared<Slides::Material>(white, black, black, black);
  auto leftMaterial = std::make_shared<Slides::Material>(red, black, black, black);
  auto rightMaterial = std::make_shared<Slides::Material>(green, black, black, black);
  auto backMaterial = std::make_shared<Slides::Material>(white, black, black, black);
  auto topMaterial = std::make_shared<Slides::Material>(white, black, black, black);
  auto botMaterial = std::make_shared<Slides::Material>(white, black, black, black);

  const auto uvChecker = image::read("assets/uvchecker.ppm");
  const auto hard = image::read("assets/hard_image_1.ppm");
  const auto eye = image::read("assets/eye.ppm");
  const auto uni = image::read("assets/unizarb.ppm");
  const auto uniMask = image::read("assets/unizarb-mask.ppm");
  const auto texUV = std::make_shared<PPMTexture>(uvChecker.buffer, 1, 1, false, false);
  const auto texHard = std::make_shared<PPMTexture>(hard.buffer, 1, 1, true, true);
  const auto texEye = std::make_shared<PPMTexture>(eye.buffer);
  const auto texUni = std::make_shared<PPMTexture>(uni.buffer);
  const auto texUniMask = std::make_shared<PPMTexture>(uniMask.buffer);
  const auto texNone = std::make_shared<ConstantTexture>(black);

  // const auto ball1Material = std::make_shared<tex::Material>(texUV, texNone, texNone);

  auto redMaterial = std::make_shared<Slides::Material>(red, black, black, black);

  const auto LBMaterial = std::make_shared<tex::Material>(texEye, texNone, texNone);
  const auto RBMaterial = std::make_shared<tex::Material>(texUni, texNone, texUniMask);
  const auto hardMaterial = std::make_shared<tex::Material>(texHard, texNone, texNone);
  const auto uvMaterial = std::make_shared<tex::Material>(texUV, texNone, texNone);

  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(-0.25, -0.7, -0.5), 0.3),
              RBMaterial));
  
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Sphere>(Point(0.25, -0.7, 0.5), 0.3),
              LBMaterial));

  auto meshImage1 = Quad(Point(0.999, 0.4, 0.4), Direction(0, 0.4, 0), Direction(0, 0, 0.4), (Direction(-1, 0, 0)));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshImage1, 0),
              hardMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshImage1, 1),
              hardMaterial));

  auto meshImage2 = Quad(Point(0.5, 0.5, -0.999), Direction(0, 0.5, 0), Direction(0.5, 0, 0), Direction(0, 0, -1));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshImage2, 0),
              uvMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshImage2, 1),
              uvMaterial));



  auto meshLeft = Quad(Point(0, 0, 1), Direction(0, 1, 0), Direction(1, 0, 0), Direction(0, 0, 1));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshLeft, 0),
              leftMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshLeft, 1),
              leftMaterial));

  auto meshRight = Quad(Point(0, 0, -1), Direction(0, 1, 0), Direction(1, 0, 0), Direction(0, 0, -1));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshRight, 0),
              rightMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshRight, 1),
              rightMaterial));

  auto meshBack = Quad(Point(1, 0, 0), Direction(0, 1, 0), Direction(0, 0, 1), (Direction(-1, 0, 0)));
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

Scene LTO(size_t width, size_t height) {
  Scene scene;

  // Camera
  Point O(2, 2, 0.8);
  Point lookAt(0, 0.3, 0);
  // Point lookAt(0, 0.05, 0);
  const auto cam = std::make_shared<PinholeCamera>(width, height, O, lookAt, 4+3);
  scene.set(cam);

  // Colors
  const auto none = Direction(0, 0, 0);
  const auto white = Direction(.9, .9, .9);
  const auto gray1 = Spectrum(0.467, 0.467, 0.467);
  // const auto gray2 = Spectrum(0.667, 0.663, 0.678);

  // Image
  const auto woodImage = image::read("assets/wood.ppm");
  scene.set(std::make_shared<PPMTexture>(image::read("assets/sky.ppm").buffer));

  // Textures
  const auto texWood = std::make_shared<PPMTexture>(woodImage.buffer);
  const auto texNone = std::make_shared<ConstantTexture>(none);

  // Materials
  const auto emitter = std::make_shared<Slides::Material>(none, none, none, white*0.5);
  const auto whiteMaterial = std::make_shared<Slides::Material>(white, none, none, none);
  const auto LTOMaterial = std::make_shared<Slides::Material>(none, Direction(1, 1, 1)*0.96-white*1, white, none);
  const auto woodMaterial = std::make_shared<tex::Material>(texWood, texNone, texNone);

  auto texNoise1 = std::make_shared<NoiseTexture>(0.3, 1.3+0.3, 1+1, 9+10, Direction(1,1,1), none);
  auto grayCte = std::make_shared<ConstantTexture>(gray1);
  const auto ballMaterial = std::make_shared<tex::Material>(grayCte, texNoise1, texNone);

  auto meshEmitter = Quad(Point(0, 3.2, 0), Direction(-1, 0, 0), Direction(0, 0, -1), Direction(0, 1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshEmitter, 0),
              emitter));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshEmitter, 1),
              emitter));

  auto meshFloor = Quad(Point(0, 0, 0), Direction(-1.5, 0, 0), Direction(0, 0, -1.5), Direction(0, 1, 0));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshFloor, 0),
              woodMaterial));
  scene.add(std::make_unique<GeometricPrimitive>(
              std::make_shared<Triangle>(meshFloor, 1),
              woodMaterial));

  scene.add(std::make_unique<GeometricPrimitive>(
            std::make_shared<Sphere>(Point(0, 0.3, 0.0), 0.19),
            ballMaterial));
  

  simply::PLYFile LTO("assets/orb.ply");
  auto meshLTO = std::make_shared<TriangleMesh>(
    Mat4::rotate(M_PI/2.0, 0, 1, 0) *
    Mat4::scale(2, 2, 2),
    LTO);
  
  for (size_t i = 0; i < meshLTO->nTriangles; i++)
    scene.add(
      std::make_unique<GeometricPrimitive>(
        std::make_shared<Triangle>(meshLTO, i),
        LTOMaterial));

  return scene;
}

#endif // SCENES_H_
