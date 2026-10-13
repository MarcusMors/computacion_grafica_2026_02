#ifndef __DRAW2D_HPP__
#define __DRAW2D_HPP__

// ============================================================================
// Draw2D.hpp — flat 2D drawing on top of the window: lines, DASHED lines,
// translucent polygons and text. No 3D, no depth, no textures.
//
//     Draw2D draw;                                     // in init() (needs the GL context)
//     ...
//     draw.begin(framebuffer_width, framebuffer_height);
//     draw.polygon({ a, b, c }, colors::yellow, 0.25f);          // translucent fill
//     draw.line(a, b, colors::white, 2.0f);                      // 2 pixels thick
//     draw.dashed_line(a, b, colors::white, 2.0f, 8.0f, 6.0f);   // 8 px dash, 6 px gap
//     draw.text("-3/2", { 100, 40 }, colors::red);               // 5x7 bitmap font
//     draw.end();                                                // sends everything to the GPU
//
// ALL COORDINATES ARE PIXELS of the framebuffer: (0,0) is the BOTTOM-LEFT
// corner, x grows right, y grows up. Everything is submitted as triangles
// (thick lines are thin rectangles), so line width never depends on driver
// support for glLineWidth, and dashes are exact in pixels.
//
// Shapes are painted in the order you call them (later ones on top).
//
// TEXT: a built-in 5x7 pixel font with the characters
//     0-9  A-Z (lowercase is drawn as uppercase)  / - + = . , : ( ) space
// `scale` is the size of one font pixel in screen pixels (2 => 10x14 glyphs).
// ============================================================================

#include <glad/gl.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <vector>

#include "Buffers.hpp"
#include "Colors.hpp"
#include "_cpp.hpp"// Program

// ---- tiny Vec2 algebra (Vec2 itself is in Buffers.hpp) ----------------------------------
inline Vec2 operator+(const Vec2 &a, const Vec2 &b) { return { a.x + b.x, a.y + b.y }; }
inline Vec2 operator-(const Vec2 &a, const Vec2 &b) { return { a.x - b.x, a.y - b.y }; }
inline Vec2 operator*(const Vec2 &a, float s) { return { a.x * s, a.y * s }; }
inline float length(const Vec2 &a) { return std::sqrt(a.x * a.x + a.y * a.y); }

enum class Text_Align { left, center, right };

class Draw2D
{
public:
  // glyph size in font pixels
  static constexpr int GLYPH_W = 5;
  static constexpr int GLYPH_H = 7;

  Draw2D()
  {
    Shader_Pipeline pipeline;
    pipeline.vertex = Shader_Source(std::string_view{ vertex_source });
    pipeline.fragment = Shader_Source(std::string_view{ fragment_source });
    program_.attach(pipeline);

    vao_.attach_vertices(vbo_, vertices_, GL_DYNAMIC_DRAW);// empty for now; binds the VAO
    vao_.attribute(0, &Vertex2D::pos);  // layout(location = 0)
    vao_.attribute(1, &Vertex2D::color);// layout(location = 1)
  }

  Draw2D(const Draw2D &) = delete;// owns GL objects
  Draw2D &operator=(const Draw2D &) = delete;

  // Start a frame of 2D drawing for a framebuffer of this size (in pixels).
  void begin(int width, int height)
  {
    width_ = static_cast<float>(width);
    height_ = static_cast<float>(height);
    vertices_.clear();
  }

  // ---- shapes -------------------------------------------------------------------------
  void line(Vec2 a, Vec2 b, const Vec3 &color, float thickness = 1.5f, float alpha = 1.0f)
  {
    const Vec2 d = b - a;
    const float len = length(d);
    if (len < 1e-4f) { return; }
    const Vec2 n{ -d.y / len * thickness * 0.5f, d.x / len * thickness * 0.5f };// half-thickness normal
    quad(a + n, a - n, b - n, b + n, color, alpha);
  }

  // A line made of dashes: `dash` pixels drawn, `gap` pixels skipped, repeated.
  void dashed_line(Vec2 a, Vec2 b, const Vec3 &color, float thickness, float dash, float gap, float alpha = 1.0f)
  {
    const Vec2 d = b - a;
    const float len = length(d);
    if (len < 1e-4f) { return; }
    const Vec2 dir = d * (1.0f / len);
    for (float t = 0.0f; t < len; t += dash + gap) {
      const float t_end = std::min(t + dash, len);
      line(a + dir * t, a + dir * t_end, color, thickness, alpha);
    }
  }

  // Filled CONVEX polygon (corners in order, either direction).
  void polygon(const std::vector<Vec2> &corners, const Vec3 &color, float alpha = 1.0f)
  {
    for (std::size_t i = 1; i + 1 < corners.size(); ++i) {
      triangle(corners[0], corners[i], corners[i + 1], color, alpha);
    }
  }
  void polygon(std::initializer_list<Vec2> corners, const Vec3 &color, float alpha = 1.0f)
  {
    polygon(std::vector<Vec2>(corners), color, alpha);
  }

  // Square `size` pixels wide, centered on `center` (handy for dots / markers).
  void dot(Vec2 center, float size, const Vec3 &color, float alpha = 1.0f)
  {
    const float h = size * 0.5f;
    quad({ center.x - h, center.y - h }, { center.x + h, center.y - h },
         { center.x + h, center.y + h }, { center.x - h, center.y + h }, color, alpha);
  }

  // Outline of the rectangle with corners `lo` (bottom-left) and `hi` (top-right).
  void rect_outline(Vec2 lo, Vec2 hi, const Vec3 &color, float thickness = 1.5f)
  {
    const float h = thickness * 0.5f;// extend the sides so the corners are closed
    line({ lo.x - h, lo.y }, { hi.x + h, lo.y }, color, thickness);
    line({ lo.x - h, hi.y }, { hi.x + h, hi.y }, color, thickness);
    line({ lo.x, lo.y }, { lo.x, hi.y }, color, thickness);
    line({ hi.x, lo.y }, { hi.x, hi.y }, color, thickness);
  }

  // ---- text -------------------------------------------------------------------------------
  static float text_width(std::string_view s, int scale = 2)
  {
    return s.empty() ? 0.0f : static_cast<float>((static_cast<int>(s.size()) * (GLYPH_W + 1) - 1) * scale);
  }
  static float text_height(int scale = 2) { return static_cast<float>(GLYPH_H * scale); }

  // `pos` is the BOTTOM of the text (its baseline); x depends on `align`.
  void text(std::string_view s, Vec2 pos, const Vec3 &color, int scale = 2, Text_Align align = Text_Align::left,
            float alpha = 1.0f)
  {
    const float w = text_width(s, scale);
    float x = pos.x;
    if (align == Text_Align::center) { x -= w * 0.5f; }
    if (align == Text_Align::right) { x -= w; }
    x = std::floor(x);// keep font pixels on whole screen pixels: crisp glyphs
    const float y0 = std::floor(pos.y);
    const float s_px = static_cast<float>(scale);

    for (char ch : s) {
      const auto &rows = glyph(ch);
      for (int row = 0; row < GLYPH_H; ++row) {
        for (int col = 0; col < GLYPH_W; ++col) {
          if ((rows[static_cast<std::size_t>(row)] >> (GLYPH_W - 1 - col)) & 1) {
            const float px = x + static_cast<float>(col) * s_px;
            const float py = y0 + static_cast<float>(GLYPH_H - 1 - row) * s_px;// row 0 is the TOP row
            quad({ px, py }, { px + s_px, py }, { px + s_px, py + s_px }, { px, py + s_px }, color, alpha);
          }
        }
      }
      x += static_cast<float>(GLYPH_W + 1) * s_px;
    }
  }

  // Send everything drawn since begin() to the GPU. Alpha blending is on only inside this call.
  void end()
  {
    if (vertices_.empty()) { return; }

    const GLboolean depth_was_on = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    program_.set_uniform("u_resolution", width_, height_);
    vao_.bind();
    vbo_.upload(vertices_, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));

    glDisable(GL_BLEND);
    if (depth_was_on) { glEnable(GL_DEPTH_TEST); }
  }

private:
  struct Vertex2D
  {
    Vec2 pos;
    Vec4 color;
  };

  void vertex(Vec2 p, const Vec3 &c, float a) { vertices_.push_back({ p, { c.x, c.y, c.z, a } }); }
  void triangle(Vec2 a, Vec2 b, Vec2 c, const Vec3 &color, float alpha)
  {
    vertex(a, color, alpha);
    vertex(b, color, alpha);
    vertex(c, color, alpha);
  }
  void quad(Vec2 a, Vec2 b, Vec2 c, Vec2 d, const Vec3 &color, float alpha)
  {
    triangle(a, b, c, color, alpha);
    triangle(a, c, d, color, alpha);
  }

  // 5x7 font: 7 rows, top to bottom; in each row bit 4 is the LEFTMOST pixel.
  static const std::array<std::uint8_t, 7> &glyph(char ch)
  {
    struct Entry { char c; std::array<std::uint8_t, 7> rows; };
    static constexpr Entry table[] = {
      { '0', { 0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110 } },
      { '1', { 0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110 } },
      { '2', { 0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111 } },
      { '3', { 0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110 } },
      { '4', { 0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010 } },
      { '5', { 0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110 } },
      { '6', { 0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110 } },
      { '7', { 0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000 } },
      { '8', { 0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110 } },
      { '9', { 0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100 } },
      { '/', { 0b00001, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b10000 } },
      { '-', { 0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000 } },
      { '+', { 0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000 } },
      { '=', { 0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000 } },
      { '.', { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100 } },
      { ',', { 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b00100, 0b01000 } },
      { ':', { 0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000 } },
      { '(', { 0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010 } },
      { ')', { 0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000 } },
      { 'A', { 0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001 } },
      { 'B', { 0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110 } },
      { 'C', { 0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110 } },
      { 'D', { 0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110 } },
      { 'E', { 0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111 } },
      { 'F', { 0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000 } },
      { 'G', { 0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111 } },
      { 'H', { 0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001 } },
      { 'I', { 0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110 } },
      { 'J', { 0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100 } },
      { 'K', { 0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001 } },
      { 'L', { 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111 } },
      { 'M', { 0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001 } },
      { 'N', { 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001 } },
      { 'O', { 0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 } },
      { 'P', { 0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000 } },
      { 'Q', { 0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101 } },
      { 'R', { 0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001 } },
      { 'S', { 0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110 } },
      { 'T', { 0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100 } },
      { 'U', { 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 } },
      { 'V', { 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100 } },
      { 'W', { 0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010 } },
      { 'X', { 0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001 } },
      { 'Y', { 0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100 } },
      { 'Z', { 0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111 } },
    };
    static constexpr std::array<std::uint8_t, 7> blank{};

    if (ch >= 'a' && ch <= 'z') { ch = static_cast<char>(ch - 'a' + 'A'); }
    for (const Entry &e : table) {
      if (e.c == ch) { return e.rows; }
    }
    return blank;// space and anything unknown
  }

  static constexpr std::string_view vertex_source = R"(#version 430 core
layout(location = 0) in vec2 a_pos;     // pixels, origin bottom-left
layout(location = 1) in vec4 a_color;   // rgba
uniform vec2 u_resolution;              // framebuffer size in pixels
out vec4 v_color;
void main(void)
{
  v_color = a_color;
  gl_Position = vec4(a_pos / u_resolution * 2.0 - 1.0, 0.0, 1.0);
}
)";

  static constexpr std::string_view fragment_source = R"(#version 430 core
in vec4 v_color;
out vec4 color;
void main(void)
{
  color = v_color;
}
)";

  Program program_;
  Vertex_Array vao_;
  Buffer vbo_;
  std::vector<Vertex2D> vertices_;
  float width_{ 1.0f };
  float height_{ 1.0f };
};

#endif// __DRAW2D_HPP__
