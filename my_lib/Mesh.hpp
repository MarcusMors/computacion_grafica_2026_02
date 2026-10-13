#ifndef __MESH_HPP__
#define __MESH_HPP__

// ============================================================================
// Mesh.hpp — CPU-side geometry: vertices, triangles, outline edges, plus the
// two measurements the course asks for (centroid, bounding box).
//
// Nothing here touches OpenGL, so a Mesh can be built, inspected and tested
// without a window. Gpu_Mesh (Renderer.hpp) uploads it to the GPU.
//
// FLAT COLOR PER FACE
// -------------------
// A vertex holds ONE color. If a corner is shared by two faces of different
// colors, it can't hold both. So every face gets its OWN copies of its
// corners (a box = 6 faces * 4 = 24 vertices, not 8). The rasterizer then
// interpolates between identical colors => each face is a single plain color,
// with no gradients and no lighting code.
// ============================================================================

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <set>
#include <span>
#include <utility>
#include <vector>

#include "Colors.hpp"
#include "Mat4.hpp"

// Named colors (colors::white, colors::dark_brown, ...) live in Colors.hpp.

struct Mesh_Vertex
{
  Vec3 pos;
  Vec3 color;
};

// ----------------------------------------------------------------------------
// Aabb — AXIS-ALIGNED bounding box: the smallest box with edges parallel to
// the X, Y, Z axes that contains all the points.
// ----------------------------------------------------------------------------
struct Aabb
{
  Vec3 min{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
  Vec3 max{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

  bool empty() const { return min.x > max.x; }
  Vec3 center() const { return empty() ? Vec3{ 0, 0, 0 } : (min + max) * 0.5f; }
  Vec3 size() const { return empty() ? Vec3{ 0, 0, 0 } : max - min; }

  Aabb &extend(const Vec3 &p)
  {
    min = { std::min(min.x, p.x), std::min(min.y, p.y), std::min(min.z, p.z) };
    max = { std::max(max.x, p.x), std::max(max.y, p.y), std::max(max.z, p.z) };
    return *this;
  }

  Aabb &extend(const Aabb &other)
  {
    if (!other.empty()) { extend(other.min); extend(other.max); }
    return *this;
  }

  // The 8 corners.
  std::array<Vec3, 8> corners() const
  {
    return { Vec3{ min.x, min.y, min.z }, Vec3{ max.x, min.y, min.z }, Vec3{ max.x, max.y, min.z }, Vec3{ min.x, max.y, min.z },
             Vec3{ min.x, min.y, max.z }, Vec3{ max.x, min.y, max.z }, Vec3{ max.x, max.y, max.z }, Vec3{ min.x, max.y, max.z } };
  }

  // The box that encloses this box after `m` is applied (the 8 corners are
  // moved, then re-boxed). It is axis-aligned again, so after a rotation it is
  // NOT the tightest possible box (that would be an oriented box).
  Aabb transformed(const Mat4 &m) const
  {
    Aabb r;
    if (empty()) { return r; }
    for (const Vec3 &c : corners()) { r.extend(transform_point(m, c)); }
    return r;
  }
};

// ----------------------------------------------------------------------------
// Mesh
// ----------------------------------------------------------------------------
class Mesh
{
public:
  std::vector<Mesh_Vertex> vertices;
  std::vector<std::uint32_t> triangles;// 3 indices per filled triangle
  std::vector<std::uint32_t> edges;    // 2 indices per outline segment

  // Add one flat-colored, CONVEX polygon. `corners` must be counter-clockwise
  // as seen from OUTSIDE the solid (that is the face's front side).
  // Creates its own vertex copies, fills it with a triangle fan and registers
  // its boundary as outline edges (an edge shared with another face is stored
  // only once).
  void add_polygon(std::span<const Vec3> corners, const Vec3 &face_color)
  {
    if (corners.size() < 3) { return; }

    const auto first = static_cast<std::uint32_t>(vertices.size());
    for (const Vec3 &c : corners) { vertices.push_back({ c, face_color }); }

    const auto n = static_cast<std::uint32_t>(corners.size());
    for (std::uint32_t i = 1; i + 1 < n; ++i) {
      triangles.insert(triangles.end(), { first, first + i, first + i + 1 });
    }

    for (std::uint32_t i = 0; i < n; ++i) {
      const std::uint32_t j = (i + 1) % n;
      const auto key = edge_key(corners[i], corners[j]);
      if (edge_keys_.insert(key).second) { edges.insert(edges.end(), { first + i, first + j }); }
    }
  }
  void add_polygon(std::initializer_list<Vec3> corners, const Vec3 &face_color)
  {
    add_polygon(std::span<const Vec3>{ corners.begin(), corners.size() }, face_color);
  }

  // Centroid = arithmetic MEAN of the distinct corner points of the figure.
  // (Corners that appear in several faces count once — otherwise a box corner,
  // which exists 3 times in the vertex list, would pull the result.)
  // NOTE: for a pyramid this is NOT its center of mass (that is h/4 above the
  // base); it is the average of its 5 corners.
  Vec3 centroid() const
  {
    std::set<Point_Key> seen;
    Vec3 sum{ 0, 0, 0 };
    float count = 0.0f;
    for (const Mesh_Vertex &v : vertices) {
      if (seen.insert(point_key(v.pos)).second) { sum += v.pos; count += 1.0f; }
    }
    return count > 0.0f ? sum / count : Vec3{ 0, 0, 0 };
  }

  // Smallest axis-aligned box containing every vertex (in the mesh's own space).
  Aabb bounds() const
  {
    Aabb b;
    for (const Mesh_Vertex &v : vertices) { b.extend(v.pos); }
    return b;
  }

private:
  using Point_Key = std::array<long long, 3>;

  // Positions are compared on a 1e-4 grid so points that were computed
  // separately (e.g. sphere seam) still count as the same point.
  static Point_Key point_key(const Vec3 &p)
  {
    return { std::llround(p.x * 10000.0f), std::llround(p.y * 10000.0f), std::llround(p.z * 10000.0f) };
  }
  static std::pair<Point_Key, Point_Key> edge_key(const Vec3 &a, const Vec3 &b)
  {
    const Point_Key ka = point_key(a), kb = point_key(b);
    return ka < kb ? std::make_pair(ka, kb) : std::make_pair(kb, ka);
  }

  std::set<std::pair<Point_Key, Point_Key>> edge_keys_;
};

#endif// __MESH_HPP__
