#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include "Buffers.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>


#include "LOGGING.hpp"
#if ENABLE_LOGGING
#include <csignal>
#endif


// A single point at the center of the window (normalized device coordinates:
// x and y in [-1, 1], so (0,0) is the exact center).
struct Vertex { Vec2 pos; };
constexpr Vec2 POINT{ 0.0f, 0.0f };

Program *program_ptr;
Vertex_Array *point_ptr;
Buffer *vbo_ptr;

// ============================================================================
// KEYBOARD INPUT — the event-driven way: a callback.
// GLFW calls this function EVERY TIME a key is pressed, released or repeated.
//   key     -> which key (GLFW_KEY_A, GLFW_KEY_0, ... — layout-independent)
//   action  -> GLFW_PRESS / GLFW_RELEASE / GLFW_REPEAT
//   mods    -> shift/ctrl/alt held (bitmask)
// ============================================================================
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS) { return; }// ignore releases and key-repeat

  const bool is_letter = (key >= GLFW_KEY_A && key <= GLFW_KEY_Z);
  const bool is_digit  = (key >= GLFW_KEY_0 && key <= GLFW_KEY_9);
  if (not (is_letter || is_digit)) { return; }// only alphanumeric for now

  // glfwGetKeyName gives the character for the user's keyboard layout.
  const char *name = glfwGetKeyName(key, scancode);

  std::cout << "key '" << (name != nullptr ? name : "?")
            << "' pressed -> point at (" << POINT.x << ", " << POINT.y << ")"
            << std::endl;
}

void init(GLFWwindow *window)
{
  program_ptr = new Program{};
  program_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment.glsl"
  } });

  // geometry: ONE vertex at the center
  const std::vector<Vertex> vertices = { { { POINT.x, POINT.y } } };

  point_ptr = new Vertex_Array{};
  vbo_ptr = new Buffer{};

  point_ptr->attach_vertices(*vbo_ptr, vertices);
  point_ptr->attribute(0, &Vertex::pos);
}

void display(GLFWwindow *window, double currentTime)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  program_ptr->use();
  glPointSize(15.0f);// a GL point is 1 pixel by default — make it visible
  point_ptr->draw_arrays(GL_Draw::POINTS, 1, 0);

  glfwSwapBuffers(window);
}

int main()
{
#if ENABLE_LOGGING
  std::signal(SIGINT, HandleInterruptSignal);
#endif

#if ENABLE_LOGGING
  auto path = log_path();
  log_file.open(path);
  if (!log_file) { cerr << "couldn't open \"" << path << "\" for writing" << endl; }
#define LOG(x) log_file << x << " ";
#else
#define LOG(x)
#endif

  if (glfwInit() == 0) {
    cerr << "Failed to initialize GLFW" << endl;
    return -1;
  }

  request_opengl_version();// 4.3 core profile

  GLFWwindow *window = glfwCreateWindow({ .width = 600, .height = 600, .title = "Project 09 - keyboard input" });
  if (window == nullptr) {
    cerr << "Failed to create window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (gladLoadGL(glfwGetProcAddress) == 0) { exit(EXIT_FAILURE); }

  // REGISTER the callback: from now on GLFW calls key_callback() for us
  // (inside glfwPollEvents) whenever a key event happens on this window.
  glfwSetKeyCallback(window, key_callback);

  init(window);

  glfwSwapInterval(1);// enable vsync

  while (not glfwWindowShouldClose(window)) {
    const double currentTime = glfwGetTime();
    display(window, currentTime);
    glfwPollEvents();// key_callback() is invoked from HERE
  }

  // Alternative you will see in other programs (not used here): POLLING.
  // Instead of a callback, ask every frame "is this key down right now?":
  //     if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { ... }
  // Polling is better for continuous movement (held keys); the callback is
  // better for one-shot actions (print once per press, like here).

  delete point_ptr;
  delete vbo_ptr;
  delete program_ptr;
  glfwDestroyWindow(window);
  glfwTerminate();

  exit(EXIT_SUCCESS);

#if ENABLE_LOGGING
  log_file.close();
#endif

  return 0;
}
