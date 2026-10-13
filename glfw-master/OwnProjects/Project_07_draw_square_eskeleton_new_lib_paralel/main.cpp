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


// The vertex data lives HERE (CPU side, an ordinary std::vector).
// The VAO never holds vertices — it only remembers how to interpret the VBO.
// Vec2 pos -> attribute() deduces: 2 components, GL_FLOAT, stride/offset free.
struct Vertex { Vec2 pos; };

// Objects are created with new/delete (not globals) because their RAII
// constructors call glGen*, which requires an active OpenGL context.
Program *program_blue_ptr;
Program *program_red_ptr;
Vertex_Array *square_ptr;
Buffer *vbo_ptr;
Index_Buffer<uint32_t> *ebo_ptr;// uint32_t -> GL_UNSIGNED_INT, deduced

void init(GLFWwindow *window)
{
  program_blue_ptr = new Program{};
  program_blue_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment.glsl"
  } });

  program_red_ptr = new Program{};
  program_red_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment_red.glsl"
  } });

  // ---- geometry: an indexed quad (VBO + EBO) --------------------------------
  const std::vector<Vertex> vertices = {
    { { 0.25f,  0.25f} },// 0: top-right
    { { 0.25f, -0.25f} },// 1: bottom-right
    { {-0.25f, -0.25f} },// 2: bottom-left
    { {-0.25f,  0.25f} },// 3: top-left
  };

  // 4 vertices, but 6 indices: each shared corner is reused, not duplicated.
  const std::vector<uint32_t> indices = { 0, 1, 2, 3,   3, 0, 1, 2 };

  square_ptr = new Vertex_Array{};
  vbo_ptr = new Buffer{};
  ebo_ptr = new Index_Buffer<uint32_t>{};

  square_ptr->attach_vertices(*vbo_ptr, vertices);
  // attribute LOCATION 0 connects to layout(location = 0) in vertex.glsl.
  // A second attribute would use another location, e.g.:
  //     struct Vertex { Vec2 pos; Vec3 color; };
  //     square_ptr->attribute(1, &Vertex::color);   // layout(location = 1)
  square_ptr->attribute(0, &Vertex::pos);
  square_ptr->attach_indices(*ebo_ptr, indices);// EBO recorded inside the VAO
}

void display(GLFWwindow *window, double currentTime)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);// set clear color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear the screen

  
  program_blue_ptr->use();
  square_ptr->draw_elements(GL_Draw::LINES, 4, 0);
  program_red_ptr->use();
  square_ptr->draw_elements(GL_Draw::LINES, 4, 4);


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
  delete square_ptr;
  delete vbo_ptr;
  delete ebo_ptr;
  delete program_blue_ptr;
  delete program_red_ptr;
  glfwDestroyWindow(window);
  glfwTerminate();

  exit(EXIT_SUCCESS);

#if ENABLE_LOGGING
  log_file.close();
#endif

  return 0;
}
