#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdio>

// Laptops with two GPUs (iGPU + dGPU): uncomment to request the discrete GPU,
// otherwise Windows may run this on the power-saving one and under-report.
// extern "C" {
//   __declspec(dllexport) unsigned long NvOptimusEnablement = 1;            // NVIDIA
//   __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;     // AMD
// }

static GLFWwindow *create_hidden_context(int major, int minor, bool core_profile)
{
  glfwDefaultWindowHints();                     // reset hints from any previous attempt
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);     // never show a window
  if (major > 0) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    if (core_profile) { glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); }
  }
  return glfwCreateWindow(64, 64, "version check", nullptr, nullptr);
}

int main()
{
  if (glfwInit() == 0) { std::fprintf(stderr, "glfwInit failed\n"); return 1; }

  // ---- answer 1: NO version hints -> driver gives its highest version ----
  GLFWwindow *window = create_hidden_context(0, 0, false);
  if (window == nullptr) {
    std::fprintf(stderr, "Could not create ANY OpenGL context.\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  if (gladLoadGL(glfwGetProcAddress) == 0) {
    std::fprintf(stderr, "glad failed to load function pointers.\n");
    return 1;
  }

  std::printf("GPU:      %s\n", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  std::printf("Vendor:   %s\n", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
  std::printf("Version:  %s   <-- full driver string\n",
              reinterpret_cast<const char *>(glGetString(GL_VERSION)));

  GLint major = 0, minor = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &major);   // GL_MAJOR/MINOR_VERSION exist since GL 3.0
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  std::printf("Maximum OpenGL version (compatibility profile): %d.%d\n", major, minor);
  std::printf("Maximum GLSL version: %s\n",
              reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

  glfwDestroyWindow(window);

  // ---- answer 2: highest CORE profile, probed 4.6 downwards ---------------
  constexpr int candidates[][2] = {
    { 4, 6 }, { 4, 5 }, { 4, 4 }, { 4, 3 }, { 4, 2 }, { 4, 1 }, { 4, 0 }, { 3, 3 }, { 3, 2 },
  };
  for (const auto &[maj, min] : candidates) {
    window = create_hidden_context(maj, min, true);
    if (window != nullptr) {
      std::printf("Maximum OpenGL version (core profile):        %d.%d\n", maj, min);
      glfwDestroyWindow(window);
      break;
    }
  }

  glfwTerminate();
  return 0;
}