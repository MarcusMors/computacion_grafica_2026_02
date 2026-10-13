#ifndef __MAT4_HPP__
#define __MAT4_HPP__

// ============================================================================
// Mat4.hpp — minimal 3D math for this project: Vec3 operations + a 4x4 matrix.
//
// We deliberately do NOT pull in <glm>: the project only needs a handful of
// operations (perspective, look_at, translate, rotate, scale, multiply), and
// writing them once here keeps the dependency list exactly as it was.
//
// Layout: COLUMN-MAJOR, m[column * 4 + row] — the same convention OpenGL and
// glm use, so the array can be handed to glUniformMatrix4fv(..., GL_FALSE, ...)
// without any transposition.
//
// 3D points use HOMOGENEOUS coordinates (x, y, z, w): w = 1 for a position
// (translation applies), w = 0 for a direction (translation is ignored).
//
// Two ways to build a transform — pick whichever reads better:
//
//   // chained (each call applies "after" the previous ones, like glm):
//   Mat4 model = Mat4::identity().translate({ 1, 0, 0 })
//                                .rotate(Axis::Y, radians(45.0f))
//                                .scale(2.0f);
//   // free functions (same result):
//   Mat4 model = mat4_translate({ 1, 0, 0 }) * mat4_rotate(Axis::Y, ...) * mat4_scale(2.0f);
// ============================================================================

#include <cmath>
#include <numbers>

#include "Buffers.hpp"// Vec2 / Vec3 / Vec4 live here already

// ----------------------------------------------------------------------------
// Angles and axes
// ----------------------------------------------------------------------------
inline constexpr float radians(float degrees) { return degrees * (std::numbers::pi_v<float> / 180.0f); }
inline constexpr float degrees(float radians_) { return radians_ * (180.0f / std::numbers::pi_v<float>); }

// The six rotation directions: a positive angle turns counter-clockwise when
// you look from the axis tip towards the origin (right-hand rule).
// NEG_* is the same axis turned the opposite way (angle is negated).
enum class Axis { X, Y, Z, NEG_X, NEG_Y, NEG_Z };

// Gimbal-lock guard: any Euler "pitch" (the middle rotation) is kept inside
// +/- this many degrees, so it can never reach the +/-90 degrees where two
// axes line up and one degree of freedom is lost. Used by Camera and Transform.
inline constexpr float MAX_PITCH_DEG = 89.0f;

// ----------------------------------------------------------------------------
// Vec3 free functions (Vec3 itself is a plain struct in Buffers.hpp; these
// give it the algebra a camera needs without changing that struct).
// ----------------------------------------------------------------------------
inline Vec3 operator+(const Vec3 &a, const Vec3 &b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline Vec3 operator-(const Vec3 &a, const Vec3 &b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline Vec3 operator*(const Vec3 &a, float s) { return { a.x * s, a.y * s, a.z * s }; }
inline Vec3 operator*(float s, const Vec3 &a) { return a * s; }

inline Vec3 operator-(const Vec3 &a) { return { -a.x, -a.y, -a.z }; }
inline Vec3 operator/(const Vec3 &a, float s) { return { a.x / s, a.y / s, a.z / s }; }
inline Vec3 &operator+=(Vec3 &a, const Vec3 &b) { a = a + b; return a; }
inline Vec3 &operator-=(Vec3 &a, const Vec3 &b) { a = a - b; return a; }
inline Vec3 &operator*=(Vec3 &a, float s) { a = a * s; return a; }
inline bool operator==(const Vec3 &a, const Vec3 &b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
// Component-wise product (used to apply a non-uniform scale to a point).
inline Vec3 operator*(const Vec3 &a, const Vec3 &b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }

inline float dot(const Vec3 &a, const Vec3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(const Vec3 &a, const Vec3 &b)
{
  return { a.y * b.z - a.z * b.y,
           a.z * b.x - a.x * b.z,
           a.x * b.y - a.y * b.x };
}

inline float length(const Vec3 &a) { return std::sqrt(dot(a, a)); }

inline Vec3 normalize(const Vec3 &a)
{
  const float len = length(a);
  return (len > 0.0f) ? a * (1.0f / len) : Vec3{ 0.0f, 0.0f, 0.0f };
}

// ----------------------------------------------------------------------------
// Mat4 — column-major 4x4 matrix.
// ----------------------------------------------------------------------------
struct Mat4
{
  float m[16]{};// m[column * 4 + row]

  // Raw pointer for glUniformMatrix4fv (transpose = GL_FALSE: already column-major).
  const float *data() const { return m; }

  static Mat4 identity()
  {
    Mat4 r;
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
  }

  // Element access by (row, column), e.g. m(0, 3) is the x translation.
  float &operator()(int row, int col) { return m[col * 4 + row]; }
  float operator()(int row, int col) const { return m[col * 4 + row]; }

  // ---- chainable transforms: *this = *this * T  (glm-style) ---------------
  // Defined below, after the free mat4_* functions they forward to.
  Mat4 &translate(const Vec3 &v);
  Mat4 &rotate(Axis axis, float angle_radians);
  Mat4 &rotate(float angle_radians, const Vec3 &axis);
  Mat4 &scale(const Vec3 &s);
  Mat4 &scale(float uniform);
};

// r = a * b  (apply b first, then a — same reading order as GLSL)
inline Mat4 operator*(const Mat4 &a, const Mat4 &b)
{
  Mat4 r;
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      float sum = 0.0f;
      for (int k = 0; k < 4; ++k) {
        sum += a.m[k * 4 + row] * b.m[col * 4 + k];
      }
      r.m[col * 4 + row] = sum;
    }
  }
  return r;
}

// ----------------------------------------------------------------------------
// The classic transforms (equivalent to glm::perspective / glm::lookAt /
// glm::translate / glm::rotate / glm::scale).
// ----------------------------------------------------------------------------

// Vertical field of view in RADIANS. Right-handed, clip z in [-1, 1] (OpenGL).
inline Mat4 mat4_perspective(float fov_y_radians, float aspect, float near_plane, float far_plane)
{
  const float t = 1.0f / std::tan(fov_y_radians * 0.5f);
  Mat4 r;// all zeros
  r.m[0] = t / aspect;
  r.m[5] = t;
  r.m[10] = (far_plane + near_plane) / (near_plane - far_plane);
  r.m[11] = -1.0f;
  r.m[14] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
  return r;
}

// Camera matrix: eye looks towards center, with `up` as the approximate up.
inline Mat4 mat4_look_at(const Vec3 &eye, const Vec3 &center, const Vec3 &up)
{
  const Vec3 f = normalize(center - eye);// forward
  const Vec3 s = normalize(cross(f, up));// side
  const Vec3 u = cross(s, f);            // true up

  Mat4 r = Mat4::identity();
  // Rotation part: rows are s, u, -f (column-major storage).
  r.m[0] = s.x;  r.m[4] = s.y;  r.m[8]  = s.z;
  r.m[1] = u.x;  r.m[5] = u.y;  r.m[9]  = u.z;
  r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
  // Translation part.
  r.m[12] = -dot(s, eye);
  r.m[13] = -dot(u, eye);
  r.m[14] =  dot(f, eye);
  return r;
}

// General (possibly OFF-AXIS) perspective frustum, like glFrustum / glm::frustum.
// left..right and bottom..top are the edges of the window ON THE NEAR PLANE (camera space, looking
// down -Z). If left = -right and bottom = -top the optical axis crosses the window centre
// (that is what mat4_perspective builds). Otherwise the axis crosses elsewhere: e.g. a window
// [0,1]x[0,1] (scaled by near_plane) puts the optical axis, and so the vanishing point of
// everything parallel to it, on the window's bottom-left CORNER.
// Right-handed, clip z in [-1, 1] (OpenGL).
inline Mat4 mat4_frustum(float left, float right, float bottom, float top, float near_plane, float far_plane)
{
  Mat4 r;// all zeros
  r.m[0] = 2.0f * near_plane / (right - left);
  r.m[5] = 2.0f * near_plane / (top - bottom);
  r.m[8] = (right + left) / (right - left);
  r.m[9] = (top + bottom) / (top - bottom);
  r.m[10] = -(far_plane + near_plane) / (far_plane - near_plane);
  r.m[11] = -1.0f;
  r.m[14] = -2.0f * far_plane * near_plane / (far_plane - near_plane);
  return r;
}

// Orthographic ("parallel") projection of the box left..right, bottom..top, near..far
// in camera space (looking down -Z). No perspective: size does not depend on distance.
// Right-handed, clip z in [-1, 1] (OpenGL), same as glm::ortho.
inline Mat4 mat4_ortho(float left, float right, float bottom, float top, float near_plane, float far_plane)
{
  Mat4 r = Mat4::identity();
  r.m[0] = 2.0f / (right - left);
  r.m[5] = 2.0f / (top - bottom);
  r.m[10] = -2.0f / (far_plane - near_plane);
  r.m[12] = -(right + left) / (right - left);
  r.m[13] = -(top + bottom) / (top - bottom);
  r.m[14] = -(far_plane + near_plane) / (far_plane - near_plane);
  return r;
}

inline Mat4 mat4_translate(const Vec3 &v)
{
  Mat4 r = Mat4::identity();
  r.m[12] = v.x;
  r.m[13] = v.y;
  r.m[14] = v.z;
  return r;
}

// angle in RADIANS around `axis` (axis is normalized inside).
inline Mat4 mat4_rotate(float angle_radians, const Vec3 &axis)
{
  const Vec3 a = normalize(axis);
  const float c = std::cos(angle_radians);
  const float s = std::sin(angle_radians);
  const float t = 1.0f - c;

  Mat4 r = Mat4::identity();
  // Rodrigues' rotation formula, column-major.
  r.m[0] = t * a.x * a.x + c;
  r.m[1] = t * a.x * a.y + s * a.z;
  r.m[2] = t * a.x * a.z - s * a.y;

  r.m[4] = t * a.x * a.y - s * a.z;
  r.m[5] = t * a.y * a.y + c;
  r.m[6] = t * a.y * a.z + s * a.x;

  r.m[8]  = t * a.x * a.z + s * a.y;
  r.m[9]  = t * a.y * a.z - s * a.x;
  r.m[10] = t * a.z * a.z + c;
  return r;
}

inline Mat4 mat4_scale(float sx, float sy, float sz)
{
  Mat4 r = Mat4::identity();
  r.m[0] = sx;
  r.m[5] = sy;
  r.m[10] = sz;
  return r;
}

// ----------------------------------------------------------------------------
// Rotation about the six signed axes. Positive angle = counter-clockwise
// looking from the axis tip towards the origin; NEG_* turns the other way.
//     mat4_rotate(Axis::Y, radians(90))      // quarter turn about +Y
//     mat4_rotate(Axis::NEG_Y, radians(90))  // quarter turn the opposite way
// ----------------------------------------------------------------------------
inline Mat4 mat4_rotate_x(float angle_radians) { return mat4_rotate(angle_radians, Vec3{ 1.0f, 0.0f, 0.0f }); }
inline Mat4 mat4_rotate_y(float angle_radians) { return mat4_rotate(angle_radians, Vec3{ 0.0f, 1.0f, 0.0f }); }
inline Mat4 mat4_rotate_z(float angle_radians) { return mat4_rotate(angle_radians, Vec3{ 0.0f, 0.0f, 1.0f }); }

inline Mat4 mat4_rotate(Axis axis, float angle_radians)
{
  switch (axis) {
    case Axis::X: return mat4_rotate_x(angle_radians);
    case Axis::Y: return mat4_rotate_y(angle_radians);
    case Axis::Z: return mat4_rotate_z(angle_radians);
    case Axis::NEG_X: return mat4_rotate_x(-angle_radians);
    case Axis::NEG_Y: return mat4_rotate_y(-angle_radians);
    case Axis::NEG_Z: return mat4_rotate_z(-angle_radians);
  }
  return Mat4::identity();
}

inline Mat4 mat4_scale(const Vec3 &s) { return mat4_scale(s.x, s.y, s.z); }
inline Mat4 mat4_scale(float uniform) { return mat4_scale(uniform, uniform, uniform); }

// ----------------------------------------------------------------------------
// Applying a matrix to homogeneous coordinates.
// ----------------------------------------------------------------------------
inline Vec4 operator*(const Mat4 &a, const Vec4 &v)
{
  return { a.m[0] * v.x + a.m[4] * v.y + a.m[8]  * v.z + a.m[12] * v.w,
           a.m[1] * v.x + a.m[5] * v.y + a.m[9]  * v.z + a.m[13] * v.w,
           a.m[2] * v.x + a.m[6] * v.y + a.m[10] * v.z + a.m[14] * v.w,
           a.m[3] * v.x + a.m[7] * v.y + a.m[11] * v.z + a.m[15] * v.w };
}

// A POSITION (w = 1): rotation, scale AND translation apply.
inline Vec3 transform_point(const Mat4 &a, const Vec3 &p)
{
  const Vec4 r = a * Vec4{ p.x, p.y, p.z, 1.0f };
  return { r.x, r.y, r.z };
}

// A DIRECTION (w = 0): rotation and scale apply, translation is ignored.
inline Vec3 transform_direction(const Mat4 &a, const Vec3 &d)
{
  const Vec4 r = a * Vec4{ d.x, d.y, d.z, 0.0f };
  return { r.x, r.y, r.z };
}

inline Mat4 &Mat4::translate(const Vec3 &v) { return *this = *this * mat4_translate(v); }
inline Mat4 &Mat4::rotate(Axis axis, float angle_radians) { return *this = *this * mat4_rotate(axis, angle_radians); }
inline Mat4 &Mat4::rotate(float angle_radians, const Vec3 &axis) { return *this = *this * mat4_rotate(angle_radians, axis); }
inline Mat4 &Mat4::scale(const Vec3 &s) { return *this = *this * mat4_scale(s); }
inline Mat4 &Mat4::scale(float uniform) { return *this = *this * mat4_scale(uniform); }

#endif// __MAT4_HPP__
