#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include "Animation.hpp"
#include "Camera.hpp"
#include "Figure.hpp"
#include "Renderer.hpp"
#include "Castle.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <memory>


struct Scene
{
  Renderer renderer;
  Camera camera;

  Castle castle;
  Sphere sphere{ 1.0f, colors::red, colors::yellow, 16, 8 };

  Pizza pizza{ 0.9f, 8, colors::orange, colors::yellow };
  Square square{ 1.4f, colors::green };
  Triangle triangle{ 1.6f, 1.6f, colors::purple };

  Scene()
  {
    camera.position = { 0.0f, 6.0f, 17.0f };
    camera.pitch_deg = -18.0f;

    sphere.transform.position = { -7.0f, 1.0f, 0.0f };
    sphere.transform.rotation_deg.x = 20.0f;

    pizza.transform.position = { -4.0f, 1.0f, 7.5f };
    square.transform.position = { 0.0f, 1.0f, 7.5f };
    triangle.transform.position = { 4.0f, 1.0f, 7.5f };
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

  // ---- animate: just assign new values to the transforms --------------------
  s.sphere.transform.rotation_deg.y = spin(currentTime, 40.0f);
  s.sphere.transform.scale = Vec3{ 1.0f, 1.0f, 1.0f } * pulse(currentTime, 0.5f, 0.15f);

  s.castle.transform.rotation_deg.y = spin(currentTime, 10.0f);

  s.pizza.transform.rotation_deg.z = spin(currentTime, 60.0f);   // spins
  s.triangle.transform.rotation_deg.y = spin(currentTime, 90.0f);// turns about its vertical axis
  s.square.transform.scale = Vec3{ pulse(currentTime, 1.0f, 0.2f), 1.0f, 1.0f };// breathes sideways

  // ---- draw -------------------------------------------------------------------
  s.renderer.begin_frame(window, s.camera, { 0.08f, 0.09f, 0.12f });
  s.castle.draw(s.renderer);
  s.sphere.draw(s.renderer);
  s.pizza.draw(s.renderer);
  s.square.draw(s.renderer);
  s.triangle.draw(s.renderer);

  glfwSwapBuffers(window);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS || scene == nullptr) { return; }
  Scene &s = *scene;

  switch (key) {
    case GLFW_KEY_O:// outline on / off
      s.renderer.outline = not s.renderer.outline;
      break;
    case GLFW_KEY_T: {// add a tower at a new spot (composite: add)
      const float n = static_cast<float>(s.castle.tower_count());
      s.castle.add_tower({ -3.0f + 1.5f * std::fmod(n, 5.0f), 0.0f, 4.2f });
      break;
    }
    case GLFW_KEY_R:// remove the last tower (composite: remove)
      s.castle.remove_last_tower();
      break;
    case GLFW_KEY_C: {// print centroid and bounding box of the castle
      const Vec3 c = s.castle.centroid();
      const Aabb b = s.castle.bounds();
      std::cout << "castle centroid (" << c.x << ", " << c.y << ", " << c.z << ")  bounds min ("
                << b.min.x << ", " << b.min.y << ", " << b.min.z << ") max ("
                << b.max.x << ", " << b.max.y << ", " << b.max.z << ")" << std::endl;
      break;
    }
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
  GLFWwindow *window = glfwCreateWindow({ .width = 1000, .height = 700, .title = "Project 11 - 3D figures" });
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

    // WASD move, Left Shift / Left Ctrl up / down, arrow keys look around.
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
