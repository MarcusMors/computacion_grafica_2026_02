#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include "Buffers.hpp"
#include "Mat4.hpp"
#include "Camera.hpp"
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
// Each vertex carries a position AND a color. The color is the SAME on all
// the vertices of a face, so the face is one plain color (no gradient).
struct Vertex { Vec3 pos; Vec3 color; };

// Outline color: change it here (r, g, b in 0..1).
constexpr Vec3 OUTLINE_COLOR{ 1.0f, 1.0f, 1.0f };// white

// Objects are created with new/delete (not globals) because their RAII
// constructors call glGen*, which requires an active OpenGL context.
Program *program_ptr;        // draws the faces (vertex color)
Program *program_border_ptr; // draws the outline (one uniform color)
Vertex_Array *pyramid_ptr;
Buffer *vbo_ptr;
Index_Buffer<uint32_t> *ebo_ptr;
Camera *camera_ptr;

// How the pyramid is stored
// -------------------------
// A vertex holds ONE color, so a corner shared by faces of different colors
// must exist once PER FACE. The pyramid is therefore 16 vertices:
//   0..3    the base (4 corners, color: base)
//   4..15   the 4 side faces, 3 vertices each (2 base corners + the apex)
//
// ONE index buffer holds two lists, back to back:
//   [ 18 triangle indices (6 triangles) | 16 line indices (8 edges) ]
// and each list is drawn with its own primitive mode:
//   draw_elements(TRIANGLES, 18, 0)    the filled faces
//   draw_elements(LINES,     16, 18)   the outline (8 edges x 2 indices)
constexpr GLsizei TRIANGLE_INDEX_COUNT = 18;
constexpr GLsizei EDGE_INDEX_COUNT = 16;

void init(GLFWwindow *window)
{
  program_ptr = new Program{};
  program_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment.glsl"
  } });

  // Same vertex shader (it already computes gl_Position from u_mvp); only the
  // fragment shader differs: it ignores the vertex color and paints u_color.
  program_border_ptr = new Program{};
  program_border_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment_border.glsl"
  } });

  // One color per face (r, g, b).
  const Vec3 base_color { 0.55f, 0.35f, 0.15f };// brown
  const Vec3 front_color{ 0.90f, 0.20f, 0.20f };// +z red
  const Vec3 right_color{ 0.20f, 0.70f, 0.30f };// +x green
  const Vec3 back_color { 0.20f, 0.40f, 0.90f };// -z blue
  const Vec3 left_color { 0.95f, 0.85f, 0.20f };// -x yellow

  const std::vector<Vertex> vertices = {
    // base (seen from below, counter-clockwise)
    /*  0 */ { { -0.5f, -0.5f, -0.5f }, base_color },
    /*  1 */ { {  0.5f, -0.5f, -0.5f }, base_color },
    /*  2 */ { {  0.5f, -0.5f,  0.5f }, base_color },
    /*  3 */ { { -0.5f, -0.5f,  0.5f }, base_color },
    // front face  (+z)
    /*  4 */ { { -0.5f, -0.5f,  0.5f }, front_color },
    /*  5 */ { {  0.5f, -0.5f,  0.5f }, front_color },
    /*  6 */ { {  0.0f,  0.5f,  0.0f }, front_color },
    // right face  (+x)
    /*  7 */ { {  0.5f, -0.5f,  0.5f }, right_color },
    /*  8 */ { {  0.5f, -0.5f, -0.5f }, right_color },
    /*  9 */ { {  0.0f,  0.5f,  0.0f }, right_color },
    // back face   (-z)
    /* 10 */ { {  0.5f, -0.5f, -0.5f }, back_color },
    /* 11 */ { { -0.5f, -0.5f, -0.5f }, back_color },
    /* 12 */ { {  0.0f,  0.5f,  0.0f }, back_color },
    // left face   (-x)
    /* 13 */ { { -0.5f, -0.5f, -0.5f }, left_color },
    /* 14 */ { { -0.5f, -0.5f,  0.5f }, left_color },
    /* 15 */ { {  0.0f,  0.5f,  0.0f }, left_color },
  };

  const std::vector<uint32_t> indices = {
    // ---- triangles (18) ----
    0, 2, 1,  0, 3, 2,// base: two triangles
    4, 5, 6,          // front  +z
    7, 8, 9,          // right  +x
    10, 11, 12,       // back   -z
    13, 14, 15,       // left   -x
    // ---- outline: 8 edges, 2 indices each (16) ----
    0, 1,  1, 2,  2, 3,  3, 0,     // the 4 base edges
    4, 6,  7, 9,  10, 12,  13, 15, // the 4 edges from a base corner up to the apex
  };

  pyramid_ptr = new Vertex_Array{};
  vbo_ptr = new Buffer{};
  ebo_ptr = new Index_Buffer<uint32_t>{};

  pyramid_ptr->attach_vertices(*vbo_ptr, vertices);
  // attribute LOCATIONs connect to layout(location = N) in vertex.glsl.
  pyramid_ptr->attribute(0, &Vertex::pos);
  pyramid_ptr->attribute(1, &Vertex::color);
  // Upload + record the EBO inside the VAO; the index TYPE (GL_UNSIGNED_INT)
  // is deduced from Index_Buffer<uint32_t>, never memorized by hand.
  pyramid_ptr->attach_indices(*ebo_ptr, indices);

  camera_ptr = new Camera{};
  camera_ptr->position = { 0.0f, 0.5f, 3.0f };// slightly above the base plane

  // depth test, otherwise triangles drawn later simply paint over nearer ones.
  glEnable(GL_DEPTH_TEST);

  // The outline lies EXACTLY on the faces, so lines and faces would "fight"
  // for the same depth (z-fighting) and the outline would flicker or vanish.
  // Polygon offset pushes only the FILLED faces slightly back, so the lines win.
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f); // (factor, units).
  // offset = factor * DZ + units * r
  // DZ =  polygon’s depth slope
  // units = scales the constant part.
  // Positive offset: increases depth, pushing polygons farther away from the camera.
  // negative offset: decreases depth, pulling it closer.
  // r: is implementation/driver-dependent and therefore offset is too.

  // can this be enabled an disabled in render loop without performance penalty? yes, Polygon offset is a rasterization state. It's a simple state update.

  glLineWidth(2.0f);
}

void display(GLFWwindow *window, double currentTime)
{
  // Keep the viewport and the aspect ratio correct even after a resize.
  int width{}, height{};
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  const float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const float angle = static_cast<float>(currentTime) * (2.0f * std::numbers::pi_v<float> / 4.0f);
  const Mat4 model = mat4_rotate(angle, Vec3{ 0.0f, 1.0f, 0.0f });

  const Mat4 mvp = camera_ptr->projection(aspect) * camera_ptr->view() * model;

  // 1) the filled faces
  program_ptr->set_uniform("u_mvp", mvp);
  pyramid_ptr->draw_elements(GL_Draw::TRIANGLES, TRIANGLE_INDEX_COUNT, 0);

  // 2) the outline: same vertices, same matrix, different program + LINES.
  //    (set_uniform() binds the program for you — no separate use() needed.)
  program_border_ptr->set_uniform("u_mvp", mvp);
  program_border_ptr->set_uniform("u_color", OUTLINE_COLOR);
  pyramid_ptr->draw_elements(GL_Draw::LINES, EDGE_INDEX_COUNT, TRIANGLE_INDEX_COUNT);

  glfwSwapBuffers(window);// present the rendered frame
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS) { return; }// ignore releases and key-repeat

  const bool is_letter = (key >= GLFW_KEY_A && key <= GLFW_KEY_Z);
  const bool is_digit  = (key >= GLFW_KEY_0 && key <= GLFW_KEY_9);
  if (not (is_letter || is_digit)) { return; }// only alphanumeric for now

  // glfwGetKeyName gives the character for the user's keyboard layout.
  const char *name = glfwGetKeyName(key, scancode);

  std::cout << "key '" << (name != nullptr ? name : "?") << "' pressed" << std::endl;
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
  GLFWwindow *window = glfwCreateWindow({ .width = 800, .height = 600, .title = "Control_1 - 3D pyramid" });
  if (window == nullptr) {
    cerr << "Failed to create window" << endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  // c. load OpenGL function pointers (glad2; the book shows glewInit() here)
  if (gladLoadGL(glfwGetProcAddress) == 0) { exit(EXIT_FAILURE); }

  glfwSetKeyCallback(window, key_callback);
  // d. calls once “init()”: put here app specific tasks.
  init(window);

  glfwSwapInterval(1);// enable vsync

  double previous_time = glfwGetTime();

  // Main loop
  while (not glfwWindowShouldClose(window)) {
    // e. repeatedly calls “display()”:  draws code into GLFWwindow.
    const double currentTime = glfwGetTime();

    // Frame time: lets the camera move at the same speed at any frame rate.
    const float dt = static_cast<float>(currentTime - previous_time);
    previous_time = currentTime;

    // WASD to move, Left Shift / Left Ctrl up / down, arrow keys to look around.
    camera_ptr->process_keyboard(window, dt);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
	glfwPollEvents();// key_callback() is invoked from HERE

	display(window, currentTime);
    glfwPollEvents();
  }

  // Clean up
  delete camera_ptr;
  delete ebo_ptr;
  delete vbo_ptr;
  delete pyramid_ptr;
  delete program_border_ptr;
  delete program_ptr;
  glfwDestroyWindow(window);
  glfwTerminate();

  exit(EXIT_SUCCESS);

#if ENABLE_LOGGING
  log_file.close();
#endif

  return 0;
}
