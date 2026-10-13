#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include "Animation.hpp"
#include "Camera.hpp"
#include "Figure.hpp"
#include "Renderer.hpp"
#include "Castle.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>


constexpr float HOP_PERIOD = 1.2f; 
constexpr float HOP_HEIGHT = 1.5f; 
constexpr float HOP_AIRTIME = 0.7f;
constexpr float HOP_STAGGER = 0.3f;

constexpr Vec3 BACKGROUND = colors::dark_grey;


struct Scene
{
  Renderer renderer;
  Camera camera;

  Castle castle;
  Box ground{ { 24.0f, 0.1f, 24.0f }, colors::dark_green };

  Scene()
  {
    camera.position = { 0.0f, 4.5f, 14.0f };
    camera.pitch_deg = -10.0f;

    ground.transform.position = { 0.0f, -0.05f, 0.0f };// top face at y = 0
    ground.outline = false;
  }
};

std::unique_ptr<Scene> scene;

void init(GLFWwindow *window)
{
  scene = std::make_unique<Scene>();
}

void display(GLFWwindow *window, double currentTime)
{
  Scene &s = *scene;

  // ---- animate ------------------------------------------------------------------

  float delay = 0.0f;
  for (Tower *tower : s.castle.towers()) {
    tower->transform.position.y = hop(currentTime, HOP_PERIOD, HOP_HEIGHT, delay, HOP_AIRTIME);
    delay += HOP_STAGGER;// the next tower starts a little later
  }

  // ---- draw -----------------------------------------------------------------------
  s.renderer.begin_frame(window, s.camera, BACKGROUND);
  s.ground.draw(s.renderer);
  s.castle.draw(s.renderer);

  glfwSwapBuffers(window);
}

// ---- keyboard, way 1: the CALLBACK (one-shot events) -----------------------------
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS || scene == nullptr) { return; }
  Scene &s = *scene;

  switch (key) {
    case GLFW_KEY_O:// outline on / off
      s.renderer.outline = not s.renderer.outline;
      break;
    case GLFW_KEY_T: {// add a tower: it joins the jumping automatically
      const float n = static_cast<float>(s.castle.tower_count());
      s.castle.add_tower({ -3.0f + 1.5f * std::fmod(n, 5.0f), 0.0f, 4.2f });
      break;
    }
    case GLFW_KEY_R:// remove the last tower
      s.castle.remove_last_tower();
      break;
    default: break;
  }
}


int main()
{
  // a. initialize GLFW library,
  if (glfwInit() == 0) {
    cerr << "Failed to initialize GLFW" << endl;
    return -1;
  }

  request_opengl_version();// 4.3 core profile

  // b. instantiates a GLFWwindow,
  GLFWwindow *window = glfwCreateWindow(1000, 700, "Project 12 - jumping towers", nullptr, nullptr);
  if (window == nullptr) {
    cerr << "Failed to create window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  // c. load OpenGL function pointers
  if (gladLoadGL(glfwGetProcAddress) == 0) { exit(EXIT_FAILURE); }


  glfwSetKeyCallback(window, key_callback);

  // d. calls once "init()": put here app specific tasks.
  init(window);

  glfwSwapInterval(1);// enable vsync

  double previous_time = glfwGetTime();

  // Main loop
  while (not glfwWindowShouldClose(window)) {
    const double currentTime = glfwGetTime();
    const float dt = static_cast<float>(currentTime - previous_time);
    previous_time = currentTime;

    // ---- keyboard, way 2: POLLING (held keys) ---------------------------------
    scene->camera.process_keyboard(window, dt);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    display(window, currentTime);
    glfwPollEvents();// key_callback() is invoked from HERE
  }

  // Clean up: GPU objects first (they need the context), then the window.
  scene.reset();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
