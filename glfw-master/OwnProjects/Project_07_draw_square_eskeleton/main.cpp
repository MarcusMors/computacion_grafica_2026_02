#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>


#include "LOGGING.hpp"
#if ENABLE_LOGGING
#include <csignal>
#endif


GLuint vao[1];
const int numVAOs = 1;
Program *program_ptr;

void init(GLFWwindow *window)
{

  program_ptr = new Program{};
  program_ptr->attach({
    .vertex ={
#embed "vertex.glsl"
   }, .fragment = {
#embed "fragment.glsl"
  } });

  glGenVertexArrays(numVAOs, vao);
  glBindVertexArray(vao[0]);
  // No VBO needed because vertex shader does not read any attribute
}

float x{ 0.0f };
float inc{ 0.01f };

void display(GLFWwindow *window, double currentTime)
{
  glClear(GL_DEPTH_BUFFER_BIT);// clear the screen
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);// optional: set clear color once
  glClear(GL_COLOR_BUFFER_BIT);// clear the screen

  program_ptr->use();

  glPointSize(30.0f);
  //glDrawArrays_cpp({ .mode = GL_Draw::GL_TRIANGLE_STRIP, .start_index = 0, .count = 3 });
  glDrawArrays_cpp({ .mode = GL_Draw::LINE_LOOP, .start_index = 0, .count = 4 });
  // glDrawArrays_cpp({ .mode = GL_Draw::LINE, .start_index = 0, .count = 4 });

  // glDrawElements_cpp({ .mode = GL_Draw::LINE_LOOP, .count=4, .start_index = 0});
  // glDrawElements_cpp({ .mode = GL_Draw::LINE_LOOP, .count=4, .start_index = 0});

  // glDrawArrays_cpp({ .mode = GL_Draw::GL_LINES, .start_index = 2, .count = 3 });

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
  // glDeleteVertexArrays(numVAOs, vao);
  delete program_ptr;
  glfwDestroyWindow(window);
  glfwTerminate();

  exit(EXIT_SUCCESS);

#if ENABLE_LOGGING
  log_file.close();
#endif

  return 0;
}
