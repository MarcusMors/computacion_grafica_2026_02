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


// Animation settings: change these and recompile.
constexpr float HOP_PERIOD = 1.2f;  // seconds from the start of one jump to the next
constexpr float HOP_HEIGHT = 1.5f;  // how high a tower jumps
constexpr float HOP_AIRTIME = 0.7f; // fraction of the period spent in the air (rest = standing)
constexpr float HOP_STAGGER = 0.3f; // seconds each tower starts after the previous one

// The ball circles the castle, beating like a heart.
constexpr float ORBIT_RADIUS = 6.5f;     // distance from the castle center (the walls reach 3.2)
constexpr float ORBIT_SPEED_DEG = 35.0f; // degrees per second around the castle
constexpr float BALL_RADIUS = 1.0f;

constexpr Vec3 BACKGROUND = colors::dark_grey;


// Everything that needs the OpenGL context lives in ONE object that is created
// in init() and destroyed (reset) BEFORE the window.
// NOTE there is no Program here: the Renderer owns the shader programs
// (see Renderer.hpp), and figures only hold geometry + a Transform.
struct Scene
{
  Renderer renderer;
  Camera camera;

  Castle castle;
  Sphere ball{ BALL_RADIUS, colors::red, colors::yellow, 16, 8 };// radius, 2 colors, sectors, stacks

  // 2D figures lie in the XY plane, so they face +Z (towards the camera).
  Pizza sun{ 1.3f, 12, colors::gold, colors::orange };// 12 slices alternating gold / orange
  Triangle triangle{ 1.6f, 1.6f, colors::purple };
  // A flat slab as ground, so the jumps are easy to see.
  // (A Box instead of a rotated Square: Transform limits pitch to +/-89 degrees.)
  Box ground{ { 24.0f, 0.1f, 24.0f }, colors::dark_green };

  Scene()
  {
    camera.position = { 0.0f, 4.5f, 14.0f };
    camera.pitch_deg = -5.0f;

    ground.transform.position = { 0.0f, -0.05f, 0.0f };// top face at y = 0
    ground.outline = false;

    sun.transform.position = { 7.5f, 7.0f, -3.0f };// far right and high up
    sun.outline = false;                           // flat color only, like a sun

    triangle.transform.position = { 0.0f, 7.0f, 0.0f };// above the castle
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
  // Each tower is its own figure with its own Transform, so each one gets its
  // OWN height every frame. Only position.y is written: x and z (where the
  // tower stands) are left alone, so the jump is purely vertical.
  float delay = 0.0f;
  for (Tower *tower : s.castle.towers()) {
    tower->transform.position.y = hop(currentTime, HOP_PERIOD, HOP_HEIGHT, delay, HOP_AIRTIME);
    delay += HOP_STAGGER;// the next tower starts a little later
  }

  // The ball: position follows a circle around the castle, size follows a heartbeat.
  const float angle = radians(spin(currentTime, ORBIT_SPEED_DEG));
  const float beat = heartbeat(currentTime);
  s.ball.transform.scale = Vec3{ 1.0f, 1.0f, 1.0f } * beat;
  s.ball.transform.position = { ORBIT_RADIUS * std::cos(angle),
                                BALL_RADIUS * beat,// the ball swells upwards: its bottom stays on the ground
                                ORBIT_RADIUS * std::sin(angle) };
  s.ball.transform.rotation_deg.y = spin(currentTime, 60.0f);// also turns, so the checkers move

  // The sun: only turns in its own plane (rotation about Z).
  s.sun.transform.rotation_deg.z = spin(currentTime, 30.0f);

  // The triangle: turns about ONE axis only (the vertical Y axis).
  s.triangle.transform.rotation_deg.y = spin(currentTime, 90.0f);

  // ---- draw -----------------------------------------------------------------------
  s.renderer.begin_frame(window, s.camera, BACKGROUND);
  s.ground.draw(s.renderer);
  s.castle.draw(s.renderer);
  s.ball.draw(s.renderer);
  s.sun.draw(s.renderer);
  s.triangle.draw(s.renderer);

  glfwSwapBuffers(window);
}

// ---- keyboard, way 1: the CALLBACK (one-shot events) -----------------------------
// GLFW calls this function for us, from inside glfwPollEvents(), every time a key
// is pressed, repeated or released. It is registered in main() with
// glfwSetKeyCallback(). Good for "do this ONCE when the key goes down".
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
  GLFWwindow *window = glfwCreateWindow({ .width = 1000, .height = 700, .title = "Project 13 - jumping towers, orbiting ball" });
  if (window == nullptr) {
    cerr << "Failed to create window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  // c. load OpenGL function pointers
  if (gladLoadGL(glfwGetProcAddress) == 0) { exit(EXIT_FAILURE); }

  // REGISTER the callback: from now on GLFW calls key_callback() for us
  // (inside glfwPollEvents) whenever a key event happens on this window.
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
    // Asks "is this key down RIGHT NOW?" every frame. Camera::process_keyboard()
    // does this with glfwGetKey() for W A S D, Shift, Ctrl and the arrows.
    // No registration is needed: it is just a function we call here.
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
