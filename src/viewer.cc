#include "viewer.hh"

#ifdef VIEWER

#include "image/tonemap.hh"

Viewer::Viewer(Scene &scene_, size_t max_depth, HemisphereSampler sampler_)
  : scene(scene_), maxDepth(max_depth), sampler(sampler_), mode(Mode::IMAGE), idx(0), spp(1), camera({0}) {
  int width = scene.camera->film.getWidth();
  int height = scene.camera->film.getHeight();

  aspectRatio = static_cast<float>(width) / static_cast<float>(height);

  dstWidth = width;
  dstHeight = height;
  dstXOffset = 0;
  dstYOffset = 0;

  camera.zoom = 1.0f;

  raylib::SetConfigFlags(raylib::FLAG_WINDOW_RESIZABLE);
  raylib::InitWindow(width, height, "ver (viewer)");

  target = raylib::LoadRenderTexture(width, height);
  buffer = new raylib::Color[width * height];

  resetFront = scene.camera->forward;
  resetLeft = scene.camera->left;
  resetUp = scene.camera->up;
  resetEye = scene.camera->eye;
}

Viewer::~Viewer() {
  delete[] buffer;
  raylib::UnloadRenderTexture(target);
  raylib::CloseWindow();
}

void Viewer::run() {
  size_t width = scene.camera->film.getWidth();
  size_t height = scene.camera->film.getHeight();
      
  while (!raylib::WindowShouldClose()) {
    handleInput();
    render();

    // Copy pathtracer buffer
    auto film = scene.camera->film;
    // Scale by spp
    film.buffer /= spp;

    // Tonemap
    tonemap(film);

    // Copy to buffer
    #pragma omp parallel for
    for (size_t i = 0; i < width * height; i++) {
      const auto &px = film[i];
      uint8_t r = px.r * 255;
      uint8_t g = px.g * 255;
      uint8_t b = px.b * 255;
      buffer[i] = raylib::Color{r, g, b, 255};
    }
    // Update texture
    raylib::UpdateTexture(target.texture, buffer);

    // Draw
    // -------------------
    raylib::BeginDrawing();
      // Clear
      raylib::ClearBackground(raylib::BLACK);

      // Draw texture
      raylib::BeginMode2D(camera);
        raylib::DrawTexturePro(target.texture, 
                              {0, 0, static_cast<float>(width), static_cast<float>(height)},
                              {0, 0, dstWidth, dstHeight},
                              {-dstXOffset, -dstYOffset}, 0, raylib::WHITE);
      raylib::EndMode2D();

      // HUD
      DrawHUD();
    raylib::EndDrawing();
  }
}
  
void Viewer::render() {
  size_t width = scene.camera->film.getWidth();
  size_t height = scene.camera->film.getHeight();

  #pragma omp parallel
  for (size_t k = 0; k < 512*2; k++) {
    size_t i = idx % width;
    size_t j = idx / width;
    idx++;

    SurfaceInteraction si;
    si.t = 0;
    si.n = Direction(0, 0, 0);

    Spectrum L;
    Ray r = scene.camera->getRay(i, j);

    scene.intersect(r, si);
    L += pathtracer::Li(r, scene, maxDepth, sampler);

    scene.camera->writeColor(i, j, L);
    // scene.camera->writeNormal(i, j, si.n);
    // camera->writeDepth(i, j, si.t);

    if (idx >= width * height) {
      idx = 0;
      spp++;
    }
  }
}

void Viewer::tonemap(image::Film &film) {
  const float max = film.max();
  const float gamma = 2.2;
  image::tonemap::Gamma(gamma, max).applyTo(film);
}

void Viewer::handleInput() {
  float dt = raylib::GetFrameTime();
  static bool keyDown = false;
  bool relesead = false;

  // Change mode
  if (raylib::IsKeyPressed(raylib::KEY_TAB)) {
    if (mode == Mode::IMAGE) resetCamera2D();
    mode = (mode == Mode::IMAGE) ? Mode::GAME : Mode::IMAGE;
  }

  // Movement
  if (mode == Mode::GAME) {
    // Keyboard
    if (raylib::IsKeyUp(raylib::KEY_A) && raylib::IsKeyUp(raylib::KEY_D) &&
        raylib::IsKeyUp(raylib::KEY_W) && raylib::IsKeyUp(raylib::KEY_S) &&
        raylib::IsKeyUp(raylib::KEY_LEFT_SHIFT) && raylib::IsKeyUp(raylib::KEY_SPACE) &&
        !raylib::IsMouseButtonDown(raylib::MOUSE_BUTTON_RIGHT)) {
      if (keyDown) relesead = true;
      else relesead = false;
      keyDown = false;
    }
    if (raylib::IsKeyDown(raylib::KEY_A) || raylib::IsKeyDown(raylib::KEY_D) ||
        raylib::IsKeyDown(raylib::KEY_W) || raylib::IsKeyDown(raylib::KEY_S) ||
        raylib::IsKeyDown(raylib::KEY_LEFT_SHIFT) || raylib::IsKeyDown(raylib::KEY_SPACE) ||
        raylib::IsMouseButtonDown(raylib::MOUSE_BUTTON_RIGHT)) {
      keyDown = true;
    }
    // std::cout << "Keydown: " << keyDown << ", Released: " << relesead << std::endl;

    if (raylib::IsKeyDown(raylib::KEY_A))
      scene.camera->eye += scene.camera->left.normalize() * dt;
    if (raylib::IsKeyDown(raylib::KEY_D))
      scene.camera->eye -= scene.camera->left.normalize() * dt;
    if (raylib::IsKeyDown(raylib::KEY_W))
      scene.camera->eye += scene.camera->forward.normalize() * dt;
    if (raylib::IsKeyDown(raylib::KEY_S))
      scene.camera->eye -= scene.camera->forward.normalize() * dt;
    if (raylib::IsKeyDown(raylib::KEY_LEFT_SHIFT))
      scene.camera->eye -= Direction(0, 1, 0) * dt;
    if (raylib::IsKeyDown(raylib::KEY_SPACE))
      scene.camera->eye += Direction(0, 1, 0) * dt;

    // Mouse
    if (raylib::IsMouseButtonDown(raylib::MOUSE_BUTTON_RIGHT)) {
      raylib::Vector2 delta = raylib::GetMouseDelta();
      Mat4 rotx = Mat4::rotate(delta.x * dt, 0, 1, 0);
      Mat4 roty = Mat4::rotate(delta.y * dt, 1, 0, 0);

      scene.camera->forward = rotx * roty * scene.camera->forward;
      scene.camera->left = scene.camera->forward.cross(Direction(0, 1, 0)).normalize();
      scene.camera->up = scene.camera->left.cross(scene.camera->forward).normalize();
    }

    if (relesead) resetTarget();
  }

  if (mode == Mode::IMAGE) {
    // Zoom
    float wheel = raylib::GetMouseWheelMove();
    if (wheel != 0) {
      raylib::Vector2 mouse = raylib::GetScreenToWorld2D(raylib::GetMousePosition(), camera);
      camera.offset = raylib::GetMousePosition();
      camera.target = mouse;

      float zoom = 1.0f + 0.1f * std::abs(wheel);
      if (wheel < 0) zoom = 1.0f / zoom;
      camera.zoom *= zoom;
    }

    // Pan
    if (raylib::IsMouseButtonDown(raylib::MOUSE_BUTTON_LEFT)) {
      raylib::Vector2 delta = raylib::GetMouseDelta();
      delta = raylib::Vector2Scale(delta, -1.0f / camera.zoom);
      camera.target = raylib::Vector2Add(camera.target, delta);
    }
  }

  // Reset
  if (raylib::IsKeyPressed(raylib::KEY_R)) {
    if (mode == Mode::IMAGE) resetCamera2D();
    if (mode == Mode::GAME) {
      resetTarget();

      scene.camera->eye = resetEye;
      scene.camera->forward = resetFront;
      scene.camera->left = resetLeft;
      scene.camera->up = resetUp;
    }
  }

  // Handle resize
  if (raylib::IsWindowResized()) {
    int w = raylib::GetScreenWidth();
    int h = raylib::GetScreenHeight();

    if (w / aspectRatio > h) {
      dstWidth = h * aspectRatio;
      dstHeight = h;

      dstXOffset = (w - dstWidth) / 2;
      if (dstXOffset < 0) dstXOffset = 0;
    } else {
      dstWidth = w;
      dstHeight = w / aspectRatio;

      dstYOffset = (h - dstHeight) / 2;
      if (dstYOffset < 0) dstYOffset = 0;
    }
  }
}

void Viewer::DrawHUD() {
  std::string spp_str = "SPP: " + std::to_string(spp);
  std::string mode_str = "MODE: " + std::string((mode == Mode::IMAGE) ? "IMAGE" : "GAME");
  raylib::Color mode_color = (mode == Mode::IMAGE) ? raylib::PURPLE : raylib::BLUE;

  raylib::DrawFPS(10, 10);

  raylib::DrawText(spp_str.c_str(), 10, 35, 10, raylib::RAYWHITE);

  raylib::DrawText("R: Reset", 10, 50, 10, raylib::RAYWHITE);

  raylib::DrawText("(TAB)", 10, 65, 10, raylib::RAYWHITE);
  raylib::DrawText(mode_str.c_str(), 45, 65, 10, mode_color);

  if (mode == Mode::GAME) {
    raylib::DrawText("WASD: Move", 10, 80, 10, raylib::RAYWHITE);
    raylib::DrawText("SHIFT/SPACE: Up/Down", 10, 95, 10, raylib::RAYWHITE);
    raylib::DrawText("RMB: Rotate", 10, 110, 10, raylib::RAYWHITE);
  }

  if (mode == Mode::IMAGE) {
    raylib::DrawText("Mouse Wheel: Zoom", 10, 80, 10, raylib::RAYWHITE);
    raylib::DrawText("LMB: Pan", 10, 95, 10, raylib::RAYWHITE);
  }
}

void Viewer::resetCamera2D() {
  camera.zoom = 1.0f;
  camera.target = raylib::Vector2{0, 0};
  camera.offset = raylib::Vector2{0, 0};
}

void Viewer::resetTarget() {
  idx = 0;
  spp = 0;
  for (size_t i = 0; i < scene.camera->film.getWidth() * scene.camera->film.getHeight(); i++) {
    scene.camera->film.buffer[i] = image::Pixel{0, 0, 0};
    buffer[i] = raylib::Color{0, 0, 0, 255};
  }
}

#endif