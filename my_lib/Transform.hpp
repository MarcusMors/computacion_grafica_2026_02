#ifndef __TRANSFORM_HPP__
#define __TRANSFORM_HPP__

// ============================================================================
// Transform.hpp — where a figure is, how it is turned and how big it is.
//
//     Transform t;
//     t.position = { 2, 0, -1 };        // translate
//     t.scale    = { 1, 2, 1 };         // scale (per axis)
//     t.rotate(Axis::Y, 30.0f);         // rotate 30 degrees about +Y
//     t.rotate(Axis::NEG_X, 10.0f);     // ... and 10 degrees about -X
//     Mat4 model = t.matrix();          // = Translate * Rotate * Scale
//
// The 4x4 matrix (homogeneous coordinates) is built only when asked for, so
// the fields can be edited freely every frame (animations just assign them).
// ============================================================================

#include "Mat4.hpp"

// An orthonormal basis (three unit, mutually perpendicular vectors) that says
// where "right", "up" and "forward" point for some object.
// forward = -Z, like the Camera (OpenGL convention).
struct Frame
{
  Vec3 right{ 1.0f, 0.0f, 0.0f };
  Vec3 up{ 0.0f, 1.0f, 0.0f };
  Vec3 forward{ 0.0f, 0.0f, -1.0f };
};

struct Transform
{
  Vec3 position{ 0.0f, 0.0f, 0.0f };
  // Euler angles in DEGREES: x = pitch, y = yaw, z = roll.
  // Applied as  R = Ry(yaw) * Rx(pitch) * Rz(roll).
  Vec3 rotation_deg{ 0.0f, 0.0f, 0.0f };
  Vec3 scale{ 1.0f, 1.0f, 1.0f };

  // ---- editing helpers (all chainable) ------------------------------------
  Transform &translate(const Vec3 &delta) { position += delta; return *this; }

  // Rotate by `angle_deg` about one of the six signed axes (NEG_* = opposite way).
  Transform &rotate(Axis axis, float angle_deg)
  {
    switch (axis) {
      case Axis::X: rotation_deg.x += angle_deg; break;
      case Axis::Y: rotation_deg.y += angle_deg; break;
      case Axis::Z: rotation_deg.z += angle_deg; break;
      case Axis::NEG_X: rotation_deg.x -= angle_deg; break;
      case Axis::NEG_Y: rotation_deg.y -= angle_deg; break;
      case Axis::NEG_Z: rotation_deg.z -= angle_deg; break;
    }
    return *this;
  }

  Transform &scale_by(float factor) { scale *= factor; return *this; }

  // ---- results --------------------------------------------------------------
  // Rotation only (no translation, no scale).
  Mat4 rotation_matrix() const
  {
    // Gimbal-lock guard: the middle rotation (pitch, about X) never reaches
    // +/-90 degrees, where the yaw and roll axes would line up.
    float pitch = rotation_deg.x;
    if (pitch > MAX_PITCH_DEG) { pitch = MAX_PITCH_DEG; }
    if (pitch < -MAX_PITCH_DEG) { pitch = -MAX_PITCH_DEG; }

    return mat4_rotate_y(radians(rotation_deg.y)) * mat4_rotate_x(radians(pitch))
           * mat4_rotate_z(radians(rotation_deg.z));
  }

  // local space -> parent space:  T * R * S  (scale first, then rotate, then move)
  Mat4 matrix() const
  {
    return mat4_translate(position) * rotation_matrix() * mat4_scale(scale);
  }

  // The object's own orthonormal axes, expressed in the parent's space.
  Frame frame() const
  {
    const Mat4 r = rotation_matrix();
    return { normalize(transform_direction(r, Vec3{ 1.0f, 0.0f, 0.0f })),
             normalize(transform_direction(r, Vec3{ 0.0f, 1.0f, 0.0f })),
             normalize(transform_direction(r, Vec3{ 0.0f, 0.0f, -1.0f })) };
  }

  // Move along the object's OWN axes (not the world's):
  //     t.move_along_frame({ 0, 0, 1 });   // 1 unit towards where it is facing
  Transform &move_along_frame(const Vec3 &amounts, const Frame &f)
  {
    position += f.right * amounts.x + f.up * amounts.y + f.forward * amounts.z;
    return *this;
  }
  Transform &move_along_frame(const Vec3 &amounts) { return move_along_frame(amounts, frame()); }
};

#endif// __TRANSFORM_HPP__
