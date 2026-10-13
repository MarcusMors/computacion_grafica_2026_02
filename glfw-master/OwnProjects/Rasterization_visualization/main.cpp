// ============================================================================
// Rasterization_visualization
//
// A grid of squares simulates the pixels of a screen. Pick two endpoints
// A and B with the mouse and compare how different line-rasterization
// algorithms choose which "pixels" to light:
//
//   1 - Naive:        walk x, evaluate y = m*x + b, round        (O(|dx|))
//   2 - DDA:          digital differential analyzer (float adds)
//   3 - Bresenham:    integer-only midpoint algorithm
//   4 - Diamond-exit: the rule OpenGL itself uses (diamond inside pixel)
//
// The thin RED segment is the REAL line drawn by OpenGL (GL_LINES), so you
// can see that algorithm 4 matches the hardware.
//
// Controls:
//   Left click  = set endpoint A (green)
//   Right click = set endpoint B (blue)
//   1..4        = choose algorithm
//   + / -       = grid resolution up / down
//   G           = toggle the real OpenGL line
//   ESC         = quit
// ============================================================================
#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include "Window.hpp"
#include "Buffers.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Algorithms: each returns the list of grid cells (i, j) it lights
// ---------------------------------------------------------------------------
using Cell = glm::ivec2;

// 1. Naive: for each x compute y = m*x+b and round. Swaps axes on steep slopes.
static std::vector<Cell> naive_line(Cell a, Cell b)
{
  std::vector<Cell> cells;
  const bool steep = std::abs(b.y - a.y) > std::abs(b.x - a.x);
  if (steep) { std::swap(a.x, a.y); std::swap(b.x, b.y); }
  if (a.x > b.x) { std::swap(a, b); }

  const float m = (b.x != a.x) ? float(b.y - a.y) / float(b.x - a.x) : 0.0f;
  for (int x = a.x; x <= b.x; ++x) {
    const int y = int(std::round(m * float(x - a.x) + float(a.y)));
    cells.push_back(steep ? Cell{ y, x } : Cell{ x, y });
  }
  return cells;
}

// 2. DDA: step = max(|dx|,|dy|); both coordinates advance by float increments.
static std::vector<Cell> dda_line(Cell a, Cell b)
{
  std::vector<Cell> cells;
  const int dx = b.x - a.x, dy = b.y - a.y;
  const int steps = std::max(std::abs(dx), std::abs(dy));
  if (steps == 0) { return { a }; }

  const float xi = float(dx) / steps, yi = float(dy) / steps;
  float x = float(a.x), y = float(a.y);
  for (int k = 0; k <= steps; ++k) {
    cells.push_back({ int(std::round(x)), int(std::round(y)) });
    x += xi;
    y += yi;
  }
  return cells;
}

// 3. Bresenham: integer arithmetic only, error term decides when to step.
static std::vector<Cell> bresenham_line(Cell a, Cell b)
{
  std::vector<Cell> cells;
  int x0 = a.x, y0 = a.y, x1 = b.x, y1 = b.y;
  const int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
  const int sx = (x0 < x1) ? 1 : -1;
  const int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    cells.push_back({ x0, y0 });
    if (x0 == x1 && y0 == y1) { break; }
    const int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 < dx) { err += dx; y0 += sy; }
  }
  return cells;
}

// 4. Diamond-exit rule (OpenGL spec, section "Line Segments"):
// a pixel is lit iff the ideal segment intersects the diamond
//     |x - cx| + |y - cy| <= 1/2
// inside the pixel square. Solved exactly by transforming to
// u = x + y, v = x - y (the diamond becomes an axis-aligned square)
// and clipping the segment against it (Liang-Barsky).
// Clip one coordinate of the segment q0 + t*d against [-1/2, +1/2].
// Narrows the valid t interval; returns false when the interval is empty.
static bool clip_axis(float q0, float d, float &t_enter, float &t_exit)
{
  if (d == 0.0f) { return q0 >= -0.5f && q0 <= 0.5f; }// parallel to the slab
  float t1 = (-0.5f - q0) / d;
  float t2 = (0.5f - q0) / d;
  if (t1 > t2) { std::swap(t1, t2); }
  t_enter = std::max(t_enter, t1);
  t_exit = std::min(t_exit, t2);
  return t_enter <= t_exit;
}

static bool segment_hits_diamond(glm::vec2 p0, glm::vec2 p1, glm::vec2 center)
{
  // u = x+y, v = x-y turns the diamond into an axis-aligned square.
  const auto to_uv = [](glm::vec2 p) { return glm::vec2{ p.x + p.y, p.x - p.y }; };
  const glm::vec2 q0 = to_uv(p0 - center), q1 = to_uv(p1 - center);
  const glm::vec2 d = q1 - q0;

  float t_enter = 0.0f, t_exit = 1.0f;
  if (!clip_axis(q0.x, d.x, t_enter, t_exit)) { return false; }
  if (!clip_axis(q0.y, d.y, t_enter, t_exit)) { return false; }
  return true;
}

static std::vector<Cell> diamond_exit_line(Cell a, Cell b, int grid_n)
{
  // Test against cell CENTERS (pixel centers, not corners).
  const glm::vec2 p0 = glm::vec2(a) + 0.5f, p1 = glm::vec2(b) + 0.5f;
  std::vector<Cell> cells;
  for (int j = 0; j < grid_n; ++j)
    for (int i = 0; i < grid_n; ++i)
      if (segment_hits_diamond(p0, p1, glm::vec2{ i + 0.5f, j + 0.5f })) { cells.push_back({ i, j }); }
  return cells;
}

// ---------------------------------------------------------------------------
// Rendering helpers
// ---------------------------------------------------------------------------
struct Vertex {
  float x, y;
  float r, g, b;
};

static void push_quad(std::vector<Vertex> &v, float x0, float y0, float x1, float y1, glm::vec3 c)
{
  v.insert(v.end(), { { x0, y0, c.r, c.g, c.b }, { x1, y0, c.r, c.g, c.b }, { x1, y1, c.r, c.g, c.b },
                      { x0, y0, c.r, c.g, c.b }, { x1, y1, c.r, c.g, c.b }, { x0, y1, c.r, c.g, c.b } });
}

static void push_line(std::vector<Vertex> &v, glm::vec2 p, glm::vec2 q, glm::vec3 c)
{
  v.push_back({ p.x, p.y, c.r, c.g, c.b });
  v.push_back({ q.x, q.y, c.r, c.g, c.b });
}

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------
int main()
{
  int grid_n = 16;
  Cell A{ 2, 3 }, B{ 13, 12 };
  int algorithm = 3;              // start with diamond-exit (matches the hardware)
  bool show_gl_line = true;

  const char *algo_names[] = { "Naive (O(n))", "DDA", "Bresenham", "Diamond-exit (OpenGL rule)" };
  const glm::vec3 algo_colors[] = {
    { 0.95f, 0.55f, 0.10f },      // naive  - orange
    { 0.20f, 0.75f, 0.95f },      // dda    - cyan
    { 0.65f, 0.40f, 0.95f },      // bresenham - purple
    { 0.95f, 0.85f, 0.20f },      // diamond - yellow
  };

  // --- library setup (my_lib RAII) -----------------------------------------
  Glfw_Context glfw;
  request_opengl_version();// 4.3 core profile
  Window window({ .width = 900, .height = 900, .title = "Rasterization" });
  window.make_context_current();
  Window::set_vsync(1);

  Program program{};
  program.attach({ .vertex = {
#embed "vertex.glsl"
                   }, .fragment = {
#embed "fragment.glsl"
                 } });

  Vertex_Array vao;
  Buffer vbo;
  vao.bind();
  vbo.bind();
  vao.attribute(0, 2, sizeof(Vertex), offsetof(Vertex, x));// position
  vao.attribute(1, 3, sizeof(Vertex), offsetof(Vertex, r));// color

  glDisable(GL_DEPTH_TEST);// pure 2D overlay, draw order decides
  glLineWidth(1.0f);       // 1 px: exactly what the diamond-exit rule describes

  const GLint mvp_loc = glGetUniformLocation(program.id, "uMVP");

  bool prev_left = false, prev_right = false;
  bool prev_plus = false, prev_minus = false;

  // --- main loop -------------------------------------------------------------
  while (not window.should_close()) {
    if (window.is_key_pressed(GLFW_KEY_ESCAPE)) { window.request_close(); }

    // --- input: grid resolution ---------------------------------------------
    const bool plus = window.is_key_pressed(GLFW_KEY_EQUAL) || window.is_key_pressed(GLFW_KEY_KP_ADD);
    const bool minus = window.is_key_pressed(GLFW_KEY_MINUS) || window.is_key_pressed(GLFW_KEY_KP_SUBTRACT);
    if (plus && !prev_plus && grid_n < 64) { ++grid_n; }
    if (minus && !prev_minus && grid_n > 4) { --grid_n; A = glm::min(A, Cell{ grid_n - 1 }); B = glm::min(B, Cell{ grid_n - 1 }); }
    prev_plus = plus;
    prev_minus = minus;

    // --- input: algorithm keys ------------------------------------------------
    for (int k = 0; k < 4; ++k)
      if (window.is_key_pressed(GLFW_KEY_1 + k)) { algorithm = k; }
    static bool prev_g = false;
    const bool g = window.is_key_pressed(GLFW_KEY_G);
    if (g && !prev_g) { show_gl_line = !show_gl_line; }
    prev_g = g;

    // --- input: endpoints -----------------------------------------------------
    const auto [fbw, fbh] = window.framebuffer_size();
    const auto [mx, my] = window.cursor_position();
    const float margin = 0.6f;
    const float wx = -margin + float(mx / fbw) * (grid_n + 2 * margin);
    const float wy = -margin + (1.0f - float(my / fbh)) * (grid_n + 2 * margin);
    const Cell hovered{ glm::clamp(int(std::floor(wx)), 0, grid_n - 1),
                        glm::clamp(int(std::floor(wy)), 0, grid_n - 1) };

    const bool left = window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
    const bool right = window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
    if (left && !prev_left) { A = hovered; }
    if (right && !prev_right) { B = hovered; }
    prev_left = left;
    prev_right = right;

    // --- run the algorithms ---------------------------------------------------
    // Naive is ALWAYS computed: it is the dim-gray baseline every other
    // algorithm is compared against.
    const std::vector<Cell> naive_cells = naive_line(A, B);
    std::vector<Cell> cells;
    switch (algorithm) {
    case 0: cells = naive_cells; break;
    case 1: cells = dda_line(A, B); break;
    case 2: cells = bresenham_line(A, B); break;
    default: cells = diamond_exit_line(A, B, grid_n); break;
    }

    // --- build geometry ---------------------------------------------------------
    std::vector<Vertex> tri, lines;

    // baseline first (underneath): naive in dim gray, slightly LARGER quad
    // so it peeks out as a gray border where the selected algorithm agrees
    if (algorithm != 0)
      for (const Cell c : naive_cells)
        push_quad(tri, float(c.x) + 0.04f, float(c.y) + 0.04f,
                  float(c.x) + 0.96f, float(c.y) + 0.96f, { 0.45f, 0.45f, 0.48f });

    // selected algorithm on top as a smaller inset, in its own color
    for (const Cell c : cells)
      push_quad(tri, float(c.x) + 0.16f, float(c.y) + 0.16f,
                float(c.x) + 0.84f, float(c.y) + 0.84f, algo_colors[algorithm]);

    // endpoints A (green) and B (blue)
    push_quad(tri, A.x + 0.25f, A.y + 0.25f, A.x + 0.75f, A.y + 0.75f, { 0.15f, 0.9f, 0.25f });
    push_quad(tri, B.x + 0.25f, B.y + 0.25f, B.x + 0.75f, B.y + 0.75f, { 0.2f, 0.35f, 0.95f });

    // grid lines
    for (int i = 0; i <= grid_n; ++i) {
      push_line(lines, { float(i), 0.0f }, { float(i), float(grid_n) }, { 0.28f, 0.28f, 0.30f });
      push_line(lines, { 0.0f, float(i) }, { float(grid_n), float(i) }, { 0.28f, 0.28f, 0.30f });
    }

    // the REAL OpenGL line (hardware rasterization) between cell centers
    if (show_gl_line)
      push_line(lines, glm::vec2(A) + 0.5f, glm::vec2(B) + 0.5f, { 1.0f, 0.15f, 0.15f });

    // --- draw -------------------------------------------------------------------
    glClearColor(0.09f, 0.09f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    program.use();
    const glm::mat4 mvp = glm::ortho(-margin, grid_n + margin, -margin, grid_n + margin);
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

    std::vector<Vertex> all = tri;
    all.insert(all.end(), lines.begin(), lines.end());
    vbo.upload(all, GL_DYNAMIC_DRAW);

    glDrawArrays_cpp({ .mode = GL_Draw::TRIANGLES, .start_index = 0, .count = GLsizei(tri.size()) });
    glDrawArrays_cpp({ .mode = GL_Draw::LINES, .start_index = GLint(tri.size()), .count = GLsizei(lines.size()) });

    // window title = current status
    char title[192];
    if (algorithm == 0)
      std::snprintf(title, sizeof(title),
                    "Rasterization | Naive baseline (%d cells) | grid %dx%d | LMB=A RMB=B, 1-4 algo, G=GL line",
                    int(cells.size()), grid_n, grid_n);
    else
      std::snprintf(title, sizeof(title),
                    "Rasterization | %s (%d cells) vs naive baseline GRAY (%d cells) | grid %dx%d",
                    algo_names[algorithm], int(cells.size()), int(naive_cells.size()), grid_n, grid_n);
    window.set_title(title);

    window.swap_buffers();
    Window::poll_events();
  }

  return 0;
}
