#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <utility>

#include "_cpp.hpp"// P_glfwCreateWindow, request_opengl_version()

// ============================================================================
// Glfw_Context — RAII for glfwInit() / glfwTerminate().
//
// The book writes this manually in main(); forgetting glfwTerminate() or
// calling glfwInit() twice are common mistakes. With this class the library
// guarantees: created first, destroyed last, exactly once.
// ============================================================================
class Glfw_Context
{
public:
  Glfw_Context()
  {
    if (glfwInit() == 0) { throw std::runtime_error("Failed to initialize GLFW"); }
  }
  ~Glfw_Context() { glfwTerminate(); }

  Glfw_Context(const Glfw_Context &) = delete;
  Glfw_Context &operator=(const Glfw_Context &) = delete;
};

// ============================================================================
// Window — RAII for glfwCreateWindow() / glfwDestroyWindow(), plus the
// everyday operations as member functions.
//
// Typical use:
//     Glfw_Context glfw;
//     Window window({ .width = 900, .height = 900, .title = "My app" });
//     window.make_context_current();   // also loads glad, once
//     while (not window.should_close()) {
//         ...
//         window.swap_buffers();
//         window.poll_events();
//     }
// ============================================================================
class Window
{
public:
  GLFWwindow *handle{};

  explicit Window(P_glfwCreateWindow settings)
  {
    handle = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(),
                              settings.fullscreen_mode, settings.resource_sharing);
    if (handle == nullptr) { throw std::runtime_error("Failed to create GLFW window"); }
  }

  ~Window()
  {
    if (handle != nullptr) { glfwDestroyWindow(handle); }
  }

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  // A window must receive its OpenGL context before ANY glad/GL call.
  // This also loads the GL function pointers (glad) exactly once per process.
  void make_context_current()
  {
    glfwMakeContextCurrent(handle);
    static bool glad_loaded = false;
    if (not glad_loaded) {
      if (gladLoadGL(glfwGetProcAddress) == 0) {
        throw std::runtime_error("Failed to load OpenGL function pointers (glad)");
      }
      glad_loaded = true;
    }
  }

  bool should_close() const { return glfwWindowShouldClose(handle) != 0; }
  void request_close() const { glfwSetWindowShouldClose(handle, GLFW_TRUE); }

  void swap_buffers() const { glfwSwapBuffers(handle); }
  static void poll_events() { glfwPollEvents(); }

  // Enable/disable vertical sync (1 = on, 0 = off)
  static void set_vsync(int interval) { glfwSwapInterval(interval); }

  // Current drawable size in pixels (differs from window size on HiDPI)
  std::pair<int, int> framebuffer_size() const
  {
    int w{}, h{};
    glfwGetFramebufferSize(handle, &w, &h);
    return { w, h };
  }

  // Cursor position in window coordinates (origin = top-left, y down)
  std::pair<double, double> cursor_position() const
  {
    double x{}, y{};
    glfwGetCursorPos(handle, &x, &y);
    return { x, y };
  }

  bool is_key_pressed(int glfw_key) const
  { return glfwGetKey(handle, glfw_key) == GLFW_PRESS; }

  bool is_mouse_button_pressed(int glfw_button) const
  { return glfwGetMouseButton(handle, glfw_button) == GLFW_PRESS; }

  void set_title(const char *title) const { glfwSetWindowTitle(handle, title); }
};

#endif// __WINDOW_HPP__
