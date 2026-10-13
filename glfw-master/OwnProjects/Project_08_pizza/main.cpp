#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include "Buffers.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <vector>


#include "LOGGING.hpp"
#if ENABLE_LOGGING
#include <csignal>
#endif


// The vertex data lives HERE (CPU side, an ordinary std::vector).
// The VAO never holds vertices — it only remembers how to interpret the VBO.
// Vec2 pos -> attribute() deduces: 2 components, GL_FLOAT, stride/offset free.
// (When you add the translation matrix, switch to Vec3 with w = 1.0f.)
struct Vertex { Vec2 pos; };

// ---- pizza parameters: only these two (+ slice count) define the shape ----
constexpr float RADIUS = 0.5f;
constexpr Vec2  CENTER{ 0.0f, 0.0f };
constexpr int   SLICES = 8;              // more = smoother crust
constexpr int   VERTS_PER_SLICE = 3;     // (p_k, p_k+1, center) — independent triangles
constexpr int   VERTEX_COUNT = SLICES * VERTS_PER_SLICE;

// Objects are created with new/delete (not globals) because their RAII
// constructors call glGen*, which requires an active OpenGL context.
Program *program_interior_ptr;
Program *program_border_ptr;
Vertex_Array *pizza_ptr;
Buffer *vbo_ptr;
// No Index_Buffer: each slice is an independent triangle (p_k, p_k+1, center),
// so TRIANGLES and per-slice LINE_LOOP read vertices in order — draw_arrays
// is enough and an EBO would add nothing here.

void init(GLFWwindow *window)
{
  program_interior_ptr = new Program{};
  program_interior_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment_interior.glsl"
  } });

  program_border_ptr = new Program{};
  program_border_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment_border.glsl"
  } });

  // ---- geometry: GENERATED ONCE, here in init() ---------------------------
  // display() runs ~60 times per second; rebuilding the pizza there would
  // waste the CPU and (worse) the data would never reach the GPU, because
  // only attach_vertices() uploads it.
  //
  // Route C: each slice is stored as an INDEPENDENT triangle (p_k, p_k+1,
  // center) — the center is duplicated once per slice. That duplication is
  // the price for having every slice's 3 vertices contiguous, which is what
  // lets us outline each slice with its own LINE_LOOP range later.
  // NOTE: theta must be float — "int theta" would truncate every angle to 0.
  std::vector<Vertex> vertices;
  vertices.reserve(VERTEX_COUNT);

  const float pi = std::numbers::pi_v<float>;
  for (int k = 0; k < SLICES; ++k) {
    const float theta0 = 2.0f * pi * static_cast<float>(k) / SLICES;
    const float theta1 = 2.0f * pi * static_cast<float>(k + 1) / SLICES;
    vertices.push_back({ { CENTER.x + RADIUS * std::cos(theta0),
                           CENTER.y + RADIUS * std::sin(theta0) } });// p_k
    vertices.push_back({ { CENTER.x + RADIUS * std::cos(theta1),
                           CENTER.y + RADIUS * std::sin(theta1) } });// p_k+1
    vertices.push_back({ { CENTER.x, CENTER.y } });                    // center
  }

  pizza_ptr = new Vertex_Array{};
  vbo_ptr = new Buffer{};

  pizza_ptr->attach_vertices(*vbo_ptr, vertices);
  // attribute LOCATION 0 connects to layout(location = 0) in vertex.glsl.
  pizza_ptr->attribute(0, &Vertex::pos);
}

void display(GLFWwindow *window, double currentTime)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);// set clear color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear the screen

  // Interior: 3 vertices per slice, drawn as independent triangles.
  program_interior_ptr->use();
  pizza_ptr->draw_arrays(GL_Draw::TRIANGLES, VERTEX_COUNT, 0);

  // Borders: one LINE_LOOP per slice. A loop auto-closes its range, so the
  // 3 vertices (p_k, p_k+1, center) become the full triangle outline —
  // crust edge + both cut lines. (LINE_STRIP would miss the center->p_k edge.)
  program_border_ptr->use();
  glLineWidth(3.0f);
  for (int k = 0; k < SLICES; ++k) {
    pizza_ptr->draw_arrays(GL_Draw::LINE_LOOP, VERTS_PER_SLICE, k * VERTS_PER_SLICE);
  }

  glfwSwapBuffers(window);// present the rendered frame
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

// #define LOG(x) log_file << x << "\n";
#define LOG(x) log_file << x << " ";

#else

#define LOG(x)

#endif

  // a. initialize GLFW library,
  if (glfwInit() == 0) {
    cerr << "Failed to initialize GLFW" << endl;
    return -1;
  }

  request_opengl_version();// 4.3 core profile

  // b. instantiates a GLFWwindow,
  GLFWwindow *window = glfwCreateWindow({ .width = 600, .height = 600, .title = "Chapter2 - program1" });
  if (window == nullptr) {
    cerr << "Failed to create window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  // c. load OpenGL function pointers (glad2; the book shows glewInit() here)
  if (gladLoadGL(glfwGetProcAddress) == 0) { exit(EXIT_FAILURE); }

  // d. calls once “init()”: put here app specific tasks.
  init(window);

  glfwSwapInterval(1);// enable vsync

  // Main loop
  while (not glfwWindowShouldClose(window)) {
    // e. repeatedly calls “display()”:  draws code into GLFWwindow.
    const double currentTime = glfwGetTime();
    display(window, currentTime);
    glfwPollEvents();
  }

  // Clean up
  delete pizza_ptr;
  delete vbo_ptr;
  delete program_interior_ptr;
  delete program_border_ptr;
  glfwDestroyWindow(window);
  glfwTerminate();

  exit(EXIT_SUCCESS);

#if ENABLE_LOGGING
  log_file.close();
#endif

  return 0;
}



