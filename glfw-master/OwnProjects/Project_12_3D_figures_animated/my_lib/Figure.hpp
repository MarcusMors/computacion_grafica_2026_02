#ifndef __FIGURE_HPP__
#define __FIGURE_HPP__

// ============================================================================
// Figure.hpp — things you can place in a scene.
//
//   Figure          the common interface: a Transform + draw() + centroid() +
//                   bounds() + an orthonormal frame().
//   Mesh_Figure     a Figure that is one mesh (one GPU upload).
//   Box, Pyramid, Sphere, Triangle, Square, Pizza
//                   ready-made Mesh_Figures (2D ones lie in the XY plane).
//   Composite       a Figure made of other Figures. Add / remove parts; the
//                   parts move, turn and scale together with it.
//
// BUILDING A COMPOSITE (see Project_11_3D_figures/Castle.hpp):
//
//     class Tower : public Composite {
//     public:
//       Tower() {
//         add(Box{ { 0.8f, 2.0f, 0.8f }, colors::stone }, { 0, 1.0f, 0 });   // body
//         add(Pyramid{ 1.0f, 0.8f, 1.0f, colors::red },   { 0, 2.4f, 0 });   // roof
//       }
//     };
//     Castle castle;                                  // Castle : Composite, contains Towers
//     Tower &t = castle.add(Tower{}, { 3, 0, 3 });    // returns a reference to the part
//     castle.remove(t);                               // gone
//
// WHY THIS DESIGN
//   * add<T>() is a template constrained to Figures: adding something that
//     is not a Figure is a COMPILE error, and you get back the exact type T&
//     (so you can still call Tower-specific functions on it).
//   * Parts are stored behind the common Figure interface, so one list can mix
//     Boxes, Spheres and whole sub-composites, and add/remove work at run time.
//     If you never need run-time changes, build the composite once in its
//     constructor, exactly like the example above — same code, nothing else to learn.
//
// IMPORTANT: figures own GPU objects, so create them AFTER the OpenGL context
// exists (in init(), not as plain global variables).
// ============================================================================

#include <algorithm>
#include <array>
#include <type_traits>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Mat4.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "Shapes.hpp"
#include "Transform.hpp"

class Figure
{
public:
  Transform transform;// placement of this figure inside its parent

  virtual ~Figure() = default;

  // Draw this figure. `parent` is the parent's local->world matrix
  // (identity for a top-level figure).
  virtual void draw(Renderer &renderer, const Mat4 &parent = Mat4::identity()) const = 0;

  // Centroid and axis-aligned bounding box in the figure's OWN space
  // (before `transform` is applied).
  virtual Vec3 centroid() const = 0;
  virtual Aabb bounds() const = 0;

  // The same two measurements after `transform` — i.e. in the parent's space.
  Vec3 centroid_in_parent() const { return transform_point(transform.matrix(), centroid()); }
  Aabb bounds_in_parent() const { return bounds().transformed(transform.matrix()); }

  // Orthonormal basis (right, up, forward) of this figure, in the parent's space.
  virtual Frame frame() const { return transform.frame(); }

  // Move along the figure's own axes:  fig.move_along_frame({ 0, 0, 1 });
  void move_along_frame(const Vec3 &amounts) { transform.move_along_frame(amounts, frame()); }
};

// ----------------------------------------------------------------------------
// Mesh_Figure — one mesh. Copies share the same GPU data (cheap).
// ----------------------------------------------------------------------------
class Mesh_Figure : public Figure
{
public:
  bool outline{ true };               // draw this figure's edges (if the Renderer allows)
  std::optional<Vec3> outline_color;  // empty = use the Renderer's outline color

  explicit Mesh_Figure(const Mesh &mesh) : gpu_{ std::make_shared<const Gpu_Mesh>(mesh) } {}
  explicit Mesh_Figure(std::shared_ptr<const Gpu_Mesh> shared) : gpu_{ std::move(shared) } {}

  void draw(Renderer &renderer, const Mat4 &parent = Mat4::identity()) const override
  {
    renderer.draw(*gpu_, parent * transform.matrix(), outline, outline_color);
  }
  Vec3 centroid() const override { return gpu_->centroid(); }
  Aabb bounds() const override { return gpu_->bounds(); }

private:
  std::shared_ptr<const Gpu_Mesh> gpu_;
};

// ----------------------------------------------------------------------------
// Ready-made shapes. All are centered on their own origin.
// ----------------------------------------------------------------------------

// size = width (x), height (y), depth (z).
// Face colors: +Z front, -Z back, +X right, -X left, +Y top, -Y bottom.
class Box : public Mesh_Figure
{
public:
  Box(const Vec3 &size, const Vec3 &one_color) : Mesh_Figure{ make_box(size, one_color) } {}
  Box(const Vec3 &size, const std::array<Vec3, 6> &face_colors) : Mesh_Figure{ make_box(size, face_colors) } {}
};

// Square base (base_x by base_z), apex above the center.
// Face colors: base, front +Z, right +X, back -Z, left -X.
class Pyramid : public Mesh_Figure
{
public:
  Pyramid(float base_x, float height, float base_z, const Vec3 &one_color)
    : Mesh_Figure{ make_pyramid(base_x, height, base_z, one_color) } {}
  Pyramid(float base_x, float height, float base_z, const std::array<Vec3, 5> &face_colors)
    : Mesh_Figure{ make_pyramid(base_x, height, base_z, face_colors) } {}
};

// UV sphere (https://www.songho.ca/opengl/gl_sphere.html), checkerboard of two colors.
class Sphere : public Mesh_Figure
{
public:
  Sphere(float radius, const Vec3 &color_a, const Vec3 &color_b, int sectors = 24, int stacks = 12)
    : Mesh_Figure{ make_sphere(radius, sectors, stacks, color_a, color_b) } {}
  Sphere(float radius, const Vec3 &one_color, int sectors = 24, int stacks = 12)
    : Sphere{ radius, one_color, one_color, sectors, stacks } {}
};

class Triangle : public Mesh_Figure// isosceles, in the XY plane
{
public:
  Triangle(float base, float height, const Vec3 &c) : Mesh_Figure{ make_triangle(base, height, c) } {}
};

class Square : public Mesh_Figure// in the XY plane
{
public:
  Square(float side, const Vec3 &c) : Mesh_Figure{ make_square(side, c) } {}
};

class Pizza : public Mesh_Figure// in the XY plane
{
public:
  Pizza(float radius, int slices, const Vec3 &color_a, const Vec3 &color_b)
    : Mesh_Figure{ make_pizza(radius, slices, color_a, color_b) } {}
};

// ----------------------------------------------------------------------------
// Composite — a Figure made of Figures.
// ----------------------------------------------------------------------------
class Composite : public Figure
{
public:
  // Add a part (moved in). Returns a reference to the stored part (valid until
  // the part is removed or the Composite destroyed) so you can keep editing it.
  // Compile-time check: T must derive from Figure.
  template<typename T>
  T &add(T figure, const Vec3 &position = { 0.0f, 0.0f, 0.0f })
  {
    static_assert(std::is_base_of<Figure, T>::value, "Composite::add: the part must derive from Figure");
    auto part = std::make_unique<T>(std::move(figure));
    T &ref = *part;
    ref.transform.position = position;
    parts_.push_back(std::move(part));
    return ref;
  }

  // Remove a part previously returned by add(). True if it was found.
  bool remove(const Figure &part)
  {
    const auto it = std::find_if(parts_.begin(), parts_.end(), [&](const auto &p) { return p.get() == &part; });
    if (it == parts_.end()) { return false; }
    if (frame_source_ == &part) { frame_source_ = nullptr; }
    parts_.erase(it);
    return true;
  }

  void clear() { parts_.clear(); frame_source_ = nullptr; }
  std::size_t size() const { return parts_.size(); }
  bool empty() const { return parts_.empty(); }

  void draw(Renderer &renderer, const Mat4 &parent = Mat4::identity()) const override
  {
    const Mat4 world = parent * transform.matrix();
    for (const auto &part : parts_) { part->draw(renderer, world); }
  }

  // Mean of the parts' centroids (each part counts once, whatever its size),
  // in the Composite's own space.
  Vec3 centroid() const override
  {
    if (parts_.empty()) { return { 0.0f, 0.0f, 0.0f }; }
    Vec3 sum{ 0.0f, 0.0f, 0.0f };
    for (const auto &part : parts_) { sum += part->centroid_in_parent(); }
    return sum / static_cast<float>(parts_.size());
  }

  // Box that encloses every part, in the Composite's own space.
  Aabb bounds() const override
  {
    Aabb box;
    for (const auto &part : parts_) { box.extend(part->bounds_in_parent()); }
    return box;
  }

  // ---- orthonormal frame -----------------------------------------------------
  // By default a Composite has its OWN frame (from its transform). Call
  // use_frame_of(other) to take the axes of another figure instead — e.g. a
  // castle that should face the same way as its keep. `other` is NOT owned:
  // it must outlive its use here (parts of this Composite are safe: remove()
  // forgets them). Call use_own_frame() to go back.
  void use_frame_of(const Figure &other) { frame_source_ = &other; }
  void use_own_frame() { frame_source_ = nullptr; }
  Frame frame() const override { return frame_source_ ? frame_source_->frame() : Figure::frame(); }

private:
  std::vector<std::unique_ptr<Figure>> parts_;
  const Figure *frame_source_{ nullptr };
};

#endif// __FIGURE_HPP__
