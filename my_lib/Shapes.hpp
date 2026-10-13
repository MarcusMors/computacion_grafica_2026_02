#ifndef __SHAPES_HPP__
#define __SHAPES_HPP__

// ============================================================================
// Shapes.hpp — functions that BUILD meshes (CPU only, no OpenGL needed).
//
// Every shape is centered on the origin of its own space:
//   3D:  make_box, make_pyramid, make_sphere
//   2D:  make_triangle, make_square, make_pizza    (lying in the XY plane, z = 0)
// Each face is ONE plain color (see Mesh.hpp). Faces are counter-clockwise
// as seen from outside, so face culling / lighting can be switched on later.
//
// You normally don't call these directly: the figure classes in Figure.hpp
// (Box, Pyramid, Sphere, Triangle, Square, Pizza) wrap them.
// ============================================================================

#include <array>
#include <cmath>
#include <numbers>

#include "Mesh.hpp"

// ---- Box ------------------------------------------------------------------
// Face colors, in this order:  +Z front, -Z back, +X right, -X left, +Y top, -Y bottom.
inline Mesh make_box(const Vec3 &size, const std::array<Vec3, 6> &face_colors)
{
  const float x = size.x * 0.5f, y = size.y * 0.5f, z = size.z * 0.5f;
  Mesh m;
  m.add_polygon({ { -x, -y, z }, { x, -y, z }, { x, y, z }, { -x, y, z } }, face_colors[0]);   // +Z front
  m.add_polygon({ { x, -y, -z }, { -x, -y, -z }, { -x, y, -z }, { x, y, -z } }, face_colors[1]);// -Z back
  m.add_polygon({ { x, -y, z }, { x, -y, -z }, { x, y, -z }, { x, y, z } }, face_colors[2]);   // +X right
  m.add_polygon({ { -x, -y, -z }, { -x, -y, z }, { -x, y, z }, { -x, y, -z } }, face_colors[3]);// -X left
  m.add_polygon({ { -x, y, z }, { x, y, z }, { x, y, -z }, { -x, y, -z } }, face_colors[4]);   // +Y top
  m.add_polygon({ { -x, -y, -z }, { x, -y, -z }, { x, -y, z }, { -x, -y, z } }, face_colors[5]);// -Y bottom
  return m;
}
inline Mesh make_box(const Vec3 &size, const Vec3 &one_color)
{
  return make_box(size, { one_color, one_color, one_color, one_color, one_color, one_color });
}

// ---- Pyramid (square base) ------------------------------------------------
// Base is base_x by base_z, apex is straight above the base center.
// Face colors, in this order:  base (-Y), front (+Z), right (+X), back (-Z), left (-X).
inline Mesh make_pyramid(float base_x, float height, float base_z, const std::array<Vec3, 5> &face_colors)
{
  const float x = base_x * 0.5f, y = height * 0.5f, z = base_z * 0.5f;
  const Vec3 apex{ 0.0f, y, 0.0f };
  Mesh m;
  m.add_polygon({ { -x, -y, -z }, { x, -y, -z }, { x, -y, z }, { -x, -y, z } }, face_colors[0]);// base
  m.add_polygon({ { -x, -y, z }, { x, -y, z }, apex }, face_colors[1]);                       // front +Z
  m.add_polygon({ { x, -y, z }, { x, -y, -z }, apex }, face_colors[2]);                       // right +X
  m.add_polygon({ { x, -y, -z }, { -x, -y, -z }, apex }, face_colors[3]);                     // back  -Z
  m.add_polygon({ { -x, -y, -z }, { -x, -y, z }, apex }, face_colors[4]);                     // left  -X
  return m;
}
inline Mesh make_pyramid(float base_x, float height, float base_z, const Vec3 &one_color)
{
  return make_pyramid(base_x, height, base_z, { one_color, one_color, one_color, one_color, one_color });
}

// ---- Sphere (UV sphere, see https://www.songho.ca/opengl/gl_sphere.html) ----
// `sectors` = slices around the Y axis (longitude), `stacks` = rings from the
// top pole to the bottom pole (latitude). Each patch is one plain color,
// alternating color_a / color_b like a checkerboard so rotation is visible.
// The two poles are triangles; every other patch is a quad.
inline Mesh make_sphere(float radius, int sectors, int stacks, const Vec3 &color_a, const Vec3 &color_b)
{
  sectors = std::max(sectors, 3);
  stacks = std::max(stacks, 2);

  const float pi = std::numbers::pi_v<float>;
  // Point at stack i (0 = top pole) and sector j (j == sectors wraps to 0).
  auto point = [&](int i, int j) {
    const float phi = pi / 2.0f - pi * static_cast<float>(i) / static_cast<float>(stacks);// +90 .. -90 degrees
    const float theta = 2.0f * pi * static_cast<float>(j) / static_cast<float>(sectors);
    const float ring = radius * std::cos(phi);
    return Vec3{ ring * std::cos(theta), radius * std::sin(phi), -ring * std::sin(theta) };
  };

  Mesh m;
  for (int i = 0; i < stacks; ++i) {
    for (int j = 0; j < sectors; ++j) {
      const Vec3 &c = ((i + j) % 2 == 0) ? color_a : color_b;
      const Vec3 top_left = point(i, j), top_right = point(i, j + 1);
      const Vec3 bottom_left = point(i + 1, j), bottom_right = point(i + 1, j + 1);
      if (i == 0) {
        m.add_polygon({ top_left, bottom_left, bottom_right }, c);// north pole cap
      } else if (i == stacks - 1) {
        m.add_polygon({ top_left, bottom_left, top_right }, c);// south pole cap
      } else {
        m.add_polygon({ top_left, bottom_left, bottom_right, top_right }, c);
      }
    }
  }
  return m;
}

// ---- 2D shapes, in the XY plane (z = 0), facing +Z --------------------------
inline Mesh make_triangle(float base, float height, const Vec3 &c)
{
  Mesh m;
  m.add_polygon({ { -base * 0.5f, -height * 0.5f, 0.0f }, { base * 0.5f, -height * 0.5f, 0.0f }, { 0.0f, height * 0.5f, 0.0f } }, c);
  return m;
}

inline Mesh make_square(float side, const Vec3 &c)
{
  const float h = side * 0.5f;
  Mesh m;
  m.add_polygon({ { -h, -h, 0.0f }, { h, -h, 0.0f }, { h, h, 0.0f }, { -h, h, 0.0f } }, c);
  return m;
}

// The pizza of Project_08: `slices` triangles around the center; the slice
// colors alternate (color_a, color_b) — pass the same color twice for a plain disc.
inline Mesh make_pizza(float radius, int slices, const Vec3 &color_a, const Vec3 &color_b)
{
  slices = std::max(slices, 3);
  const float pi = std::numbers::pi_v<float>;
  Mesh m;
  for (int k = 0; k < slices; ++k) {
    const float t0 = 2.0f * pi * static_cast<float>(k) / static_cast<float>(slices);
    const float t1 = 2.0f * pi * static_cast<float>(k + 1) / static_cast<float>(slices);
    m.add_polygon({ { 0.0f, 0.0f, 0.0f },
                    { radius * std::cos(t0), radius * std::sin(t0), 0.0f },
                    { radius * std::cos(t1), radius * std::sin(t1), 0.0f } },
      (k % 2 == 0) ? color_a : color_b);
  }
  return m;
}

#endif// __SHAPES_HPP__
