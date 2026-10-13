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
// For 3D we carry a position AND a color per vertex; the fragment shader
// interpolates the color across each triangle, so the pyramid's faces are
// readable without any lighting code.
struct Vertex { Vec3 pos; Vec3 color; };

// Objects are created with new/delete (not globals) because their RAII
// constructors call glGen*, which requires an active OpenGL context.
Program *program_ptr;
Vertex_Array *pyramid_ptr;
Buffer *vbo_ptr;
Index_Buffer<uint32_t> *ebo_ptr;
Camera *camera_ptr;

// The pyramid is an INDEXED mesh: 5 vertices, 18 indices (6 triangles).
// Unlike the pizza (independent triangles, draw_arrays), here the base
// corners are shared by three triangles each, so an EBO avoids duplicating
// them — and it exercises Vertex_Array::attach_indices() / draw_elements().

void init(GLFWwindow *window)
{
  program_ptr = new Program{};
  program_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment.glsl"
  } });

  // ---- geometry: GENERATED ONCE, here in init() ---------------------------
  // Square base on the y = -0.5 plane (half side 0.5), apex at y = +0.5.
  // Index winding is counter-clockwise seen from OUTSIDE, so the faces are
  // correct if you later enable GL_CULL_FACE.
  const std::vector<Vertex> vertices = {
    //  base corners                      position                 color
    /* 0 */ { { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },// red
    /* 1 */ { {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f } },// green
    /* 2 */ { {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },// blue
    /* 3 */ { { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f } },// yellow
    /* 4 */ { {  0.0f,  0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f } },// white apex
  };

  const std::vector<uint32_t> indices = {
    0, 2, 1,  0, 3, 2,// base (two triangles, facing down)
    0, 1, 4,          // front  (+z)
    1, 2, 4,          // right  (+x)
    2, 3, 4,          // back   (-z)
    3, 0, 4,          // left   (-x)
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

  // A 3D scene needs the depth test, otherwise triangles drawn later simply
  // paint over nearer ones.
  glEnable(GL_DEPTH_TEST);
}

void display(GLFWwindow *window, double currentTime)
{
  // Keep the viewport and the aspect ratio correct even after a resize.
  int width{}, height{};
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  const float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;

  glClearColor(0.1f, 0.1f, 0.15f, 1.0f);// dark blue-grey background
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // model: spin the pyramid around the Y axis, one turn every 4 seconds.
  const float angle = static_cast<float>(currentTime) * (2.0f * std::numbers::pi_v<float> / 4.0f);
  const Mat4 model = mat4_rotate(angle, Vec3{ 0.0f, 1.0f, 0.0f });

  // mvp = projection * view * model  (applied right-to-left in the shader).
  const Mat4 mvp = camera_ptr->projection(aspect) * camera_ptr->view() * model;

  // A gentle brightness pulse, purely to show a FLOAT uniform next to the
  // Mat4 one: same call shape, same one-liner.
  const float pulse = 0.75f + 0.25f * std::sin(static_cast<float>(currentTime) * 2.0f);

  // ---- the seamless uniform API -------------------------------------------
  // No glGetUniformLocation in init(), no cached GLint globals, no manual
  // glUseProgram before glUniform: set_uniform() binds the program, finds
  // (and caches) the location, and uploads the value.
  program_ptr->set_uniform("u_mvp", mvp);
  program_ptr->set_uniform("u_pulse", pulse);

  pyramid_ptr->draw_elements(GL_Draw::TRIANGLES);

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
  GLFWwindow *window = glfwCreateWindow({ .width = 800, .height = 600, .title = "Control_1 - 3D pyramid" });
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

  double previous_time = glfwGetTime();

  // Main loop
  while (not glfwWindowShouldClose(window)) {
    // e. repeatedly calls “display()”:  draws code into GLFWwindow.
    const double currentTime = glfwGetTime();

    // Frame time: lets the camera move at the same speed at any frame rate.
    const float dt = static_cast<float>(currentTime - previous_time);
    previous_time = currentTime;

    // WASD + E/Q to move, arrow keys to look around.
    camera_ptr->process_keyboard(window, dt);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    display(window, currentTime);
    glfwPollEvents();
  }

  // Clean up
  delete camera_ptr;
  delete ebo_ptr;
  delete vbo_ptr;
  delete pyramid_ptr;
  delete program_ptr;
  glfwDestroyWindow(window);
  glfwTerminate();

  exit(EXIT_SUCCESS);

#if ENABLE_LOGGING
  log_file.close();
#endif

  return 0;
}
