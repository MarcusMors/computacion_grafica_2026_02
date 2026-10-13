// ============================================================================
// pedagogic_visualizations_look_at
//
// What a cube looks like from a camera placed at V, looking at a target, with
// a chosen "up" vector. The 3D -> 2D steps are done HERE, on the CPU, one
// matrix at a time, so every number can be printed and checked by hand:
//
//   world point  --view matrix (look-at)-->  camera space  --projection-->  clip
//   clip --divide by w--> NDC (-1..1) --> position inside a SQUARE frame (0..1)
//
// Two sets of rulers on the border of the frame (key M):
//   SCREEN rulers: the 2D coordinates themselves, 0..1, with a grid. The tick step is 1/N where N is
//                  the least common denominator of all the 2D coordinates (12 in the classic exercise).
//   WORLD rulers:  one ruler per world axis (x red, y green, z blue); a tick for the value t sits where
//                  the world point t*axis lands on the screen.
// All labels are exact fractions (boost::rational). When every input is rational and the camera is
// "nice" (axis-aligned), the 2D coordinates are computed in EXACT arithmetic too.
//
// The window has TWO panes:
//   LEFT  : the camera's view (what the exercise asks for).
//   RIGHT : a FREE camera you fly through the 3D world (W A S D, Left Shift / Left Ctrl, arrow keys),
//           with the x, y, z axes always drawn (ticks from -5 to 5), the cube, and the left camera
//           itself (its position V and the pyramid of what it sees). Press V to jump the free camera
//           to V looking at the target: it then shows the same picture as the left pane.
//
// Hidden edges (behind the cube) are DASHED. The faces are translucent.
// ============================================================================
#define GLAD_GL_IMPLEMENTATION
#include "_cpp.hpp"
#include "Camera.hpp"
#include "Colors.hpp"
#include "Draw2D.hpp"
#include "Mat4.hpp"
#include <GLFW/glfw3.h>
#include <boost/rational.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// ============================================================================
// EDIT HERE
// ============================================================================
// Every input number is an INTEGER in THOUSANDTHS (SCALE = 1000), so that it can
// be an exact fraction: 1000 -> 1,   500 -> 1/2,   200 -> 1/5,   3500 -> 7/2 ...
constexpr int SCALE = 1000;
using Q = boost::rational<long long>;                                // exact fraction
inline Q milli(long long thousandths) { return Q(thousandths, SCALE); }   // reduces by itself: milli(500) == 1/2
struct Q3 { Q x, y, z; };

// The window the camera sees, measured ON THE IMAGE PLANE at distance 1 in front of the camera
// (a point (X, Y, Z) in camera space lands at (X/Z, Y/Z)). A window that is not centred on the
// optical axis is an OFF-AXIS frustum.
struct Window { Q left, right, bottom, top; };

struct Setup
{
  const char *name;
  Q3 camera;          // V
  Q3 target;
  Q3 up;              // any vector not parallel to the view direction (look-at makes it perpendicular)
  Q3 cube_center;
  Q cube_half;        // the cube goes from center-half to center+half on every axis
  bool exam_labels;   // false: vertex letters as in the GeoGebra file;  true: A..D near face, E..H far face
  bool use_fov;       // true: symmetric window from FOV_DEG (not rational);  false: use `window`
  Window window;
  int camera_space_of;   // -1: a normal setup.  >= 0: THAT setup, with the whole world moved by its look-at matrix
                         // so its camera sits at the ORIGIN looking down -Z (the cube becomes a rotated cube)
};

// Press 8, 9 or 0 to switch between the setups while running. START_SETUP is the one shown first.
constexpr int START_SETUP = 0;
const std::array<Setup, 3> SETUPS{ {
  // 0: the camera at V = (2, 3, 5) looking at the origin, cube [-1,1]^3 (the GeoGebra cube)
  { "LOOK-AT FROM (2,3,5)",
    { milli(2000), milli(3000), milli(5000) }, { milli(0), milli(0), milli(0) }, { milli(0), milli(1000), milli(0) },
    { milli(0), milli(0), milli(0) }, milli(1000), false, true, { milli(0), milli(0), milli(0), milli(0) }, -1 },

  // 1: the classic exercise. The SAME cube C and the SAME camera position V = (2,3,5) as setup 0, but the camera is NOT
  //    aimed at the origin: it only looks straight down -Z (no rotation, just the translation by -V), and its picture
  //    is the upside-down / left-right-flipped one of a real pinhole (= a camera rolled 180 degrees: up = (0,-1,0)).
  //    With the window [0,1]x[0,1] on the image plane at distance 1 this is exactly
  //        u = (Vx - Px) / (Vz - Pz),   v = (Vy - Py) / (Vz - Pz)
  //    Result: A(1/4,1/2) B(3/4,1/2) C(1/4,1) D(3/4,1) E(1/6,1/3) F(1/2,1/3) G(1/6,2/3) H(1/2,2/3)
  { "EXAM: FROM (2,3,5) LOOKING -Z",
    { milli(2000), milli(3000), milli(5000) }, { milli(2000), milli(3000), milli(4000) }, { milli(0), milli(-1000), milli(0) },
    { milli(0), milli(0), milli(0) }, milli(1000), false, false,
    { milli(0), milli(1000), milli(0), milli(1000) }, -1 },

  // 2: setup 0 seen "from the inside": the look-at matrix of setup 0 is applied to the WHOLE world (cube and axes), so
  //    the camera c1 sits at the origin looking down -Z, and the cube is the rotated cube "C in camera space".
  //    The picture in the left pane is IDENTICAL to setup 0's: P * V * p  =  P * (I) * (V * p).
  { "C1 AT THE ORIGIN",
    { milli(0), milli(0), milli(0) }, { milli(0), milli(0), milli(-1000) }, { milli(0), milli(1000), milli(0) },
    { milli(0), milli(0), milli(0) }, milli(1000), false, true, { milli(0), milli(0), milli(0), milli(0) }, 0 },
} };

constexpr int TICK_STEP_MILLI = 500;     // WORLD rulers: a tick every 1/2
constexpr int AXIS_EXTENT_MILLI = 4000;  // WORLD rulers: -4 ... 4
constexpr int MAX_COMMON_DENOMINATOR = 60;   // SCREEN rulers: if the least common denominator is larger, use 12 instead
constexpr int FALLBACK_DENOMINATOR = 10;     // SCREEN rulers when the 2D coordinates are not exact fractions

constexpr float FOV_DEG = 45.0f;         // vertical field of view for setups with use_fov (orthographic matches it at the target)
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;

constexpr Vec3 BACKGROUND = colors::black;
constexpr Vec3 CUBE_FILL = colors::yellow;      // translucent
constexpr float FILL_ALPHA_FRONT = 0.30f;       // faces turned towards the camera
constexpr float FILL_ALPHA_BACK = 0.12f;        // faces turned away
constexpr Vec3 CUBE_EDGE = colors::gold;
constexpr Vec3 AXIS_X = colors::red;            // the x axis, also the horizontal screen ruler
constexpr Vec3 AXIS_Y = colors::lime;           // the y axis, also the vertical screen ruler (pure green)
constexpr Vec3 AXIS_Z = colors::royal_blue;     // pure blue (colors::blue) is hard to read on black
constexpr Vec3 FRAME_COLOR = colors::light_grey;
constexpr Vec3 GRID_COLOR = colors::grey;

// the FREE camera pane (right)
constexpr int FREE_AXIS_EXTENT_MILLI = 5000;   // axes from -5 to 5 ...
constexpr int FREE_TICK_STEP_MILLI = 1000;     // ... a tick and a label at every whole number
constexpr Vec3 FREE_BACKGROUND{ 0.04f, 0.04f, 0.07f };
constexpr Vec3 FREE_START_POSITION{ 9.0f, 6.5f, 10.0f };   // where the free camera starts (and where B brings it back)
constexpr Vec3 FREE_START_TARGET{ 0.0f, 0.0f, 0.0f };
constexpr Vec3 CAMERA_GIZMO = colors::cyan;    // how the left camera is drawn in the free view
// ============================================================================

// ---- exact fractions: helpers ------------------------------------------------------------
float to_float(const Q &q) { return boost::rational_cast<float>(q); }
Vec3 to_vec3(const Q3 &q) { return { to_float(q.x), to_float(q.y), to_float(q.z) }; }

// "3/2", "-1/2", "2", "0"   (boost::rational is always reduced, denominator > 0)
std::string fraction(const Q &q)
{
  if (q.denominator() == 1) { return std::to_string(q.numerator()); }
  return std::to_string(q.numerator()) + "/" + std::to_string(q.denominator());
}
std::string fraction(const Q3 &q) { return "(" + fraction(q.x) + ", " + fraction(q.y) + ", " + fraction(q.z) + ")"; }

// k/n WITHOUT reducing (3/12 stays 3/12); 0 and n/n are written 0 and 1.
std::string over_denominator(long long k, long long n)
{
  if (k == 0) { return "0"; }
  if (k == n) { return "1"; }
  return std::to_string(k) + "/" + std::to_string(n);
}

// 4 decimals, trailing zeros removed:  2 -> "2", 0.5 -> "0.5", 0.57735 -> "0.5774"
std::string decimal(float v, int digits = 4)
{
  char buf[32];
  std::snprintf(buf, sizeof buf, "%.*f", digits, static_cast<double>(v));
  std::string s = buf;
  if (s.find('.') != std::string::npos) {
    while (s.back() == '0') { s.pop_back(); }
    if (s.back() == '.') { s.pop_back(); }
  }
  return (s == "-0") ? "0" : s;
}
std::string decimal(const Vec3 &v, int digits = 4)
{
  return "(" + decimal(v.x, digits) + ", " + decimal(v.y, digits) + ", " + decimal(v.z, digits) + ")";
}

// ---- exact vector algebra on Q3 -------------------------------------------------------------
Q3 operator-(const Q3 &a, const Q3 &b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
Q3 operator/(const Q3 &a, const Q &s) { return { a.x / s, a.y / s, a.z / s }; }
Q dot(const Q3 &a, const Q3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Q3 cross(const Q3 &a, const Q3 &b)
{
  return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

long long isqrt(long long n)
{
  long long r = static_cast<long long>(std::sqrt(static_cast<double>(n)));
  while (r * r > n) { --r; }
  while ((r + 1) * (r + 1) <= n) { ++r; }
  return r;
}

// The square root, if it is itself a fraction (9/4 -> 3/2, but 2 -> nothing).
std::optional<Q> exact_sqrt(const Q &q)
{
  if (q < 0) { return std::nullopt; }
  const long long n = isqrt(q.numerator()), d = isqrt(q.denominator());
  if (n * n != q.numerator() || d * d != q.denominator()) { return std::nullopt; }
  return Q(n, d);
}

// ---- the cube -------------------------------------------------------------------------------
// Corner i has a sign (-1 / +1) on each axis. Two lettering schemes:
//   GeoGebra : A(+,+,+) B(-,+,+) C(+,-,+) D(-,-,+) E(+,+,-) F(-,+,-) G(+,-,-) H(-,-,-)
//   exam     : A(-,-,near) B(+,-,near) C(-,+,near) D(+,+,near) then E..H the same on the far face
//              (near = larger z, because the camera looks down -Z)
struct Cube
{
  struct Face { std::array<int, 4> corner; Vec3 normal; };   // outward normal
  struct Edge { int a, b; std::array<int, 2> face; };        // the two faces that meet at the edge

  std::array<Q3, 8> vertex;                    // exact coordinates of the original cube
  std::array<Vec3, 8> pos;                     // where the corners are IN THE SCENE (= vertex, unless transform() moved them)
  std::array<std::array<int, 3>, 8> sign;
  std::array<Face, 6> faces;
  std::array<Edge, 12> edges;

  static constexpr char label(int i) { return static_cast<char>('A' + i); }

  // Move the cube in the scene with a rigid transform (rotation + translation): corners and face normals.
  void transform(const Mat4 &m)
  {
    for (Vec3 &p : pos) { p = transform_point(m, p); }
    for (Face &f : faces) { f.normal = transform_direction(m, f.normal); }
  }

  Cube(const Q3 &center, const Q &half, bool exam_labels)
  {
    for (int i = 0; i < 8; ++i) {
      sign[i] = exam_labels ? std::array<int, 3>{ (i & 1) ? 1 : -1, (i & 2) ? 1 : -1, (i & 4) ? -1 : 1 }
                            : std::array<int, 3>{ (i & 1) ? -1 : 1, (i & 2) ? -1 : 1, (i & 4) ? -1 : 1 };
      vertex[i] = { center.x + half * sign[i][0], center.y + half * sign[i][1], center.z + half * sign[i][2] };
      pos[i] = to_vec3(vertex[i]);
    }

    // a face = the 4 corners with the same sign on one axis, listed in a cycle around the face
    int nf = 0;
    for (int axis = 0; axis < 3; ++axis) {
      for (int s : { 1, -1 }) {
        const int a = (axis + 1) % 3, b = (axis + 2) % 3;
        Face f{};
        int k = 0;
        const std::array<std::array<int, 2>, 4> cycle{ { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } } };
        for (const auto &want : cycle) {
          for (int i = 0; i < 8; ++i) {
            if (sign[i][axis] == s && sign[i][a] == want[0] && sign[i][b] == want[1]) { f.corner[k++] = i; }
          }
        }
        f.normal = { axis == 0 ? static_cast<float>(s) : 0.0f, axis == 1 ? static_cast<float>(s) : 0.0f,
                     axis == 2 ? static_cast<float>(s) : 0.0f };
        faces[nf++] = f;
      }
    }

    // an edge joins two corners that differ in exactly one sign
    int ne = 0;
    for (int i = 0; i < 8; ++i) {
      for (int j = i + 1; j < 8; ++j) {
        int differing = 0;
        for (int axis = 0; axis < 3; ++axis) { differing += (sign[i][axis] != sign[j][axis]); }
        if (differing != 1) { continue; }
        Edge e{ i, j, { -1, -1 } };
        int k = 0;
        for (int f = 0; f < 6; ++f) {
          const auto &c = faces[f].corner;
          if (std::find(c.begin(), c.end(), i) != c.end() && std::find(c.begin(), c.end(), j) != c.end()) { e.face[k++] = f; }
        }
        edges[ne++] = e;
      }
    }
  }
};

// ---- camera and projection ------------------------------------------------------------------
struct View
{
  Vec3 eye;
  Vec3 target;
  Vec3 up;                  // the CURRENT up vector (changes with the keys)
  bool perspective{ true };
};

// The three orthonormal vectors that look-at builds (see the notes: f, s, u').
struct Basis { Vec3 forward, right, up; bool valid; };

Basis camera_basis(const View &v)
{
  const Vec3 f = normalize(v.target - v.eye);
  const Vec3 c = cross(f, v.up);
  if (length(c) < 1e-4f * length(v.up) || length(f) == 0.0f) { return { f, {}, {}, false }; }
  const Vec3 s = normalize(c);          // right
  return { f, s, cross(s, f), true };   // true up u' = s x f
}

Mat4 view_matrix(const View &v) { return mat4_look_at(v.eye, v.target, v.up); }

// An orthographic projection has no vanishing and no "distance": everything keeps its size. To compare it with the
// perspective picture we give it the size the perspective picture has at `ortho_dist` (see App::ortho_distance()).
Mat4 projection_matrix(const View &v, const Setup &setup, float ortho_dist)
{
  const float dist = ortho_dist;
  if (setup.use_fov) {
    if (v.perspective) { return mat4_perspective(radians(FOV_DEG), 1.0f, NEAR_PLANE, FAR_PLANE); }
    const float half = dist * std::tan(radians(FOV_DEG) * 0.5f);
    return mat4_ortho(-half, half, -half, half, NEAR_PLANE, FAR_PLANE);
  }
  const Window &w = setup.window;
  const float l = to_float(w.left), r = to_float(w.right), b = to_float(w.bottom), t = to_float(w.top);
  if (v.perspective) {   // the window is given at distance 1; glFrustum wants it on the near plane
    return mat4_frustum(l * NEAR_PLANE, r * NEAR_PLANE, b * NEAR_PLANE, t * NEAR_PLANE, NEAR_PLANE, FAR_PLANE);
  }
  return mat4_ortho(l * dist, r * dist, b * dist, t * dist, NEAR_PLANE, FAR_PLANE);// the same window, at depth `dist`
}

struct Projected
{
  Vec3 camera_space;   // after the view matrix
  Vec2 ndc;            // -1..1 inside the frame
  bool ok;             // false if the point is behind the camera
};

Projected project(const Mat4 &view, const Mat4 &projection, const Vec3 &world)
{
  const Vec3 e = transform_point(view, world);
  const Vec4 clip = projection * Vec4{ e.x, e.y, e.z, 1.0f };
  if (clip.w < 1e-5f) { return { e, { 0, 0 }, false }; }
  return { e, { clip.x / clip.w, clip.y / clip.w }, true };
}

// ---- the square frame inside the window --------------------------------------------------------------
struct Frame
{
  Vec2 lo;        // bottom-left corner in pixels
  float side;     // the frame is a SQUARE
  int text_scale;

  Vec2 hi() const { return { lo.x + side, lo.y + side }; }
  Vec2 to_pixels(Vec2 ndc) const { return { lo.x + (ndc.x * 0.5f + 0.5f) * side, lo.y + (ndc.y * 0.5f + 0.5f) * side }; }
  // fractions of the frame (0..1), as in "A: 1/4, 1/2"
  Vec2 from_fraction(float u, float v) const { return { lo.x + u * side, lo.y + v * side }; }
};

struct Rect { float x, y, w, h; };   // a part of the window, in pixels (origin bottom-left)

// The square frame, centred in `r`, with room around it for the rulers.
Frame make_frame(const Rect &r)
{
  const float shortest = std::min(r.w, r.h);
  const int scale = std::max(1, static_cast<int>(shortest) / 400);   // 800 px pane -> text scale 2
  const float margin = 46.0f * static_cast<float>(scale);            // room for the rulers outside the frame
  const float side = std::max(50.0f, shortest - 2.0f * margin);
  return { { r.x + (r.w - side) * 0.5f, r.y + (r.h - side) * 0.5f }, side, scale };
}

// Point a Camera (yaw/pitch fly camera from my_lib) at `target` from `position`.
void aim_camera(Camera &c, const Vec3 &position, const Vec3 &target)
{
  c.position = position;
  const Vec3 f = normalize(target - position);
  c.yaw_deg = degrees(std::atan2(f.z, f.x));
  c.pitch_deg = std::clamp(degrees(std::asin(std::clamp(f.y, -1.0f, 1.0f))), -MAX_PITCH_DEG, MAX_PITCH_DEG);
}

// ---- application state ------------------------------------------------------------------------------------
enum class Ruler_Mode { screen, world };
enum class Axes_Shown { cube, camera, both };   // FREE pane: the cube's own axes, the camera's axes, or both

using Screen_Coords = std::array<std::array<Q, 2>, 8>;   // exact (U, V) in 0..1 for each corner

struct App
{
  Draw2D draw;
  int setup{ 0 };
  Cube cube;
  View view;
  View initial_view;
  Q3 up_q;                   // the up vector as exact fractions (valid while up_exact)
  bool up_exact{ true };     // false after a roll (cos/sin are not fractions)
  Ruler_Mode ruler_mode{ Ruler_Mode::screen };
  bool show_hidden{ true };
  bool show_faces{ true };
  bool show_guides{ false };
  bool show_coords{ false };
  bool common_denominator_labels{ false };
  Camera free_cam;           // the camera of the RIGHT pane
  bool show_rays{ false };   // lines from V to every vertex
  Axes_Shown axes_shown{ Axes_Shown::cube };
  Mat4 scene_from_world{ Mat4::identity() };   // where the original world (and its axes) ended up in the scene
  Vec3 cube_center{ 0.0f, 0.0f, 0.0f };        // the cube's centre in WORLD coordinates

  explicit App(int index) : cube(SETUPS[0].cube_center, SETUPS[0].cube_half, SETUPS[0].exam_labels)
  {
    select_setup(index);
    reset_free_camera();
  }

  void reset_free_camera() { aim_camera(free_cam, FREE_START_POSITION, FREE_START_TARGET); free_cam.fov_y_deg = FOV_DEG; }

  const Setup &current() const { return SETUPS[static_cast<std::size_t>(setup)]; }

  // The depth (along the view direction) of the cube's centre: the distance at which the orthographic picture has
  // the same size as the perspective one. (Not the distance to the "target": in the exam setups the target is just
  // a point one unit in front of the camera, nowhere near the cube.)
  float ortho_distance() const
  {
    Vec3 c{ 0.0f, 0.0f, 0.0f };
    for (const Vec3 &p : cube.pos) { c += p; }
    c = c / 8.0f;
    const Vec3 to_target = view.target - view.eye;
    const float d = (length(to_target) > 0.0f) ? dot(normalize(to_target), c - view.eye) : 0.0f;
    return d > NEAR_PLANE ? d : std::max(length(to_target), 1.0f);
  }

  void select_setup(int index)
  {
    setup = index;
    const Setup &s = current();
    scene_from_world = Mat4::identity();
    if (s.camera_space_of >= 0) {
      // The same scene with the WHOLE WORLD moved by the look-at matrix of another setup: its camera now sits
      // at the origin looking down -Z, up = +Y, and the cube (and the world axes) are rotated / moved to match.
      const Setup &src = SETUPS[static_cast<std::size_t>(s.camera_space_of)];
      cube = Cube(src.cube_center, src.cube_half, src.exam_labels);
      cube_center = to_vec3(src.cube_center);
      scene_from_world = mat4_look_at(to_vec3(src.camera), to_vec3(src.target), to_vec3(src.up));
      cube.transform(scene_from_world);
      view = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, true };
      up_q = Q3{ milli(0), milli(1000), milli(0) };
    } else {
      cube_center = to_vec3(s.cube_center);
      cube = Cube(s.cube_center, s.cube_half, s.exam_labels);
      view = { to_vec3(s.camera), to_vec3(s.target), to_vec3(s.up), true };
      up_q = s.up;
    }
    initial_view = view;
    up_exact = true;
  }
};
std::unique_ptr<App> app;

// ---- exact 2D coordinates -----------------------------------------------------------------------------------
// With rational inputs and a camera whose basis vectors are rational (e.g. axis-aligned), the whole
// projection can be done with FRACTIONS, so the result is exact:
//   f = (target - V)/|target - V|     s = (f x up)/|f x up|     u' = s x f       (camera basis)
//   for a corner P:  d = P - V,   x_e = s.d,  y_e = u'.d,  z_e = f.d             (camera space; z_e = depth)
//   perspective:  (x_e/z_e, y_e/z_e) is the point on the image plane at distance 1
//   orthographic: (x_e/|target - V|, y_e/|target - V|)
//   2D coordinate = position of that point inside the window:  U = (u - left)/(right - left),  V likewise.
// Returns nothing when the setup uses the (irrational) FOV window, the up vector was rolled, or a square
// root in the basis is not a fraction.
std::optional<Screen_Coords> exact_screen_coords(const App &a)
{
  const Setup &s = a.current();
  if (s.use_fov || !a.up_exact) { return std::nullopt; }

  const Q3 g = s.target - s.camera;
  const auto len_g = exact_sqrt(dot(g, g));
  if (!len_g || *len_g == 0) { return std::nullopt; }
  const Q3 f = g / *len_g;
  const Q ortho_ref = dot(f, s.cube_center - s.camera);   // orthographic reference depth: the cube's centre
  if (!a.view.perspective && ortho_ref <= 0) { return std::nullopt; }
  const Q3 c = cross(f, a.up_q);
  const auto len_c = exact_sqrt(dot(c, c));
  if (!len_c || *len_c == 0) { return std::nullopt; }
  const Q3 sv = c / *len_c;
  const Q3 uv = cross(sv, f);

  const Window &w = s.window;
  Screen_Coords out{};
  for (int i = 0; i < 8; ++i) {
    const Q3 d = a.cube.vertex[i] - s.camera;
    const Q xe = dot(sv, d), ye = dot(uv, d), ze = dot(f, d);
    if (ze <= 0) { return std::nullopt; }
    const Q depth = a.view.perspective ? ze : ortho_ref;
    out[i] = { (xe / depth - w.left) / (w.right - w.left), (ye / depth - w.bottom) / (w.top - w.bottom) };
  }
  return out;
}

// Least common denominator of all the coordinates (12 for the classic exercise).
long long common_denominator(const Screen_Coords &c)
{
  long long n = 1;
  for (const auto &p : c) {
    for (const Q &q : p) { n = boost::integer::lcm(n, q.denominator()); }
  }
  return n;
}

// ---- console output ---------------------------------------------------------------------------------------------
void print_camera(const App &a)
{
  const Basis b = camera_basis(a.view);
  const Setup &s = a.current();
  std::cout << "\n---- camera ----------------------------------------------------\n"
            << "  setup          = " << s.name << "\n"
            << "  position V     = " << decimal(a.view.eye) << "\n"
            << "  target         = " << decimal(a.view.target) << "\n"
            << "  up vector      = " << decimal(a.view.up) << "\n";
  if (b.valid) {
    std::cout << "  forward  f     = " << decimal(b.forward) << "    (target - V, normalized)\n"
              << "  right    s     = " << decimal(b.right) << "    (f x up, normalized)\n"
              << "  true up  u'    = " << decimal(b.up) << "    (s x f)\n";
  }
  std::cout << "  projection     = " << (a.view.perspective ? "PERSPECTIVE" : "ORTHOGRAPHIC");
  if (s.use_fov) {
    std::cout << (a.view.perspective ? ", vertical fov " + decimal(FOV_DEG, 1) + " deg" : std::string{}) << "\n";
  } else {
    const Window &w = s.window;
    std::cout << ", window on the image plane (distance 1): x " << fraction(w.left) << " .. " << fraction(w.right)
              << ", y " << fraction(w.bottom) << " .. " << fraction(w.top) << "\n"
              << "                   (the optical axis crosses the window at ("
              << fraction(-w.left / (w.right - w.left)) << ", " << fraction(-w.bottom / (w.top - w.bottom)) << "): "
              << ((w.left == -w.right && w.bottom == -w.top) ? "its centre" : "NOT its centre: off-axis frustum") << ")\n";
  }
  if (!a.view.perspective) {
    std::cout << "  orthographic   : same size as the perspective picture at depth " << decimal(a.ortho_distance(), 3)
              << " (the cube's centre); nothing shrinks with distance\n";
  }
}

void print_table(const App &a)
{
  const Mat4 V = view_matrix(a.view);
  const Mat4 P = projection_matrix(a.view, a.current(), a.ortho_distance());
  const auto exact = exact_screen_coords(a);

  std::cout << "\n---- cube vertices ---------------------------------------------\n"
            << "  point  world (exact)        camera space (after look-at)      2D, 0..1 inside the frame\n";
  for (int i = 0; i < 8; ++i) {
    const Projected p = project(V, P, a.cube.pos[i]);
    std::string world = fraction(a.cube.vertex[i]);   // the ORIGINAL exact coordinates of C (also in setup 2)
    std::string cam = decimal(p.camera_space, 3);
    std::string xy2d = "behind the camera";
    if (exact) {
      xy2d = fraction((*exact)[i][0]) + ", " + fraction((*exact)[i][1]);
    } else if (p.ok) {
      xy2d = decimal(p.ndc.x * 0.5f + 0.5f, 4) + ", " + decimal(p.ndc.y * 0.5f + 0.5f, 4) + "   (decimal: not exact fractions)";
    }
    world.resize(std::max<std::size_t>(world.size(), 20), ' ');
    cam.resize(std::max<std::size_t>(cam.size(), 32), ' ');
    std::cout << "   " << Cube::label(i) << "     " << world << "  " << cam << "  " << xy2d << "\n";
  }

  std::cout << "\n2D coordinates:\n";
  for (int i = 0; i < 8; ++i) {
    const Projected p = project(V, P, a.cube.pos[i]);
    std::cout << "  " << Cube::label(i) << ": ";
    if (exact) {
      std::cout << fraction((*exact)[i][0]) << ", " << fraction((*exact)[i][1]) << "\n";
    } else if (p.ok) {
      std::cout << decimal(p.ndc.x * 0.5f + 0.5f, 4) << ", " << decimal(p.ndc.y * 0.5f + 0.5f, 4) << "\n";
    } else {
      std::cout << "behind the camera\n";
    }
  }
  if (exact) {
    const long long n = common_denominator(*exact);
    std::cout << "  (exact; least common denominator = " << n << ": every coordinate is a multiple of 1/" << n << ")\n";
  } else {
    std::cout << "  (decimals: with this camera the coordinates involve square roots, so they are not fractions;\n"
                 "   setups with an axis-aligned camera and a rational window, e.g. the EXAM one (key 9), are exact)\n";
  }
}

void print_setup(const App &a)
{
  const Setup &s = a.current();
  const bool moved = s.camera_space_of >= 0;
  const Setup &src = moved ? SETUPS[static_cast<std::size_t>(s.camera_space_of)] : s;
  std::cout << "setup: " << s.name << "\n";
  if (moved) {
    std::cout << "  = the setup \"" << src.name << "\" with the WHOLE WORLD moved by its look-at matrix, so that\n"
                 "    the camera is at the origin looking down -Z. The cube is now rotated (it is C in camera space)\n"
                 "    and the original world axes were carried along (key X in the right pane shows the cube's axes).\n"
                 "    The left picture is identical to that setup's: P * V * p = P * (I) * (V * p).\n";
  }
  std::cout << "inputs (exact fractions; the code stores them as integers in thousandths):\n"
            << "  camera position V = " << fraction(src.camera) << "\n"
            << "  target            = " << fraction(src.target) << "\n"
            << "  up vector         = " << fraction(src.up) << "\n"
            << "  cube center       = " << fraction(src.cube_center) << ", half size " << fraction(src.cube_half) << "\n"
            << "  WORLD ruler ticks = every " << fraction(milli(TICK_STEP_MILLI)) << ", covering +-" << fraction(milli(AXIS_EXTENT_MILLI)) << "\n";
}

void print_keys()
{
  std::cout << "\n---- keys --------------------------------------------------------\n"
               "  8        setup 0: the camera c1 at (2,3,5) looking at the origin (the default)\n"
               "  9        setup 1: the professor's / exam setup (same cube, camera at V looking down -Z, rolled 180, off-axis window)\n"
               "  0        setup 2: c1 AT THE ORIGIN - setup 0 with the whole world moved by its look-at matrix (translation AND rotation)\n"
               "  P        toggle PERSPECTIVE / ORTHOGRAPHIC\n"
               "  M        rulers on the border: SCREEN (2D coordinates 0..1 + grid)  <->  WORLD (x, y, z axes)\n"
               "  L        show / hide the 2D coordinates next to each vertex\n"
               "  C        screen ruler labels: reduced (1/6)  <->  over the common denominator (2/12)\n"
               "  1..6     up vector = +X  -X  +Y  -Y  +Z  -Z\n"
               "  Q / E    roll: turn the up vector -5 / +5 degrees around the view direction\n"
               "  R        reset the up vector and the projection\n"
               "  H        show / hide the dashed hidden edges\n"
               "  F        show / hide the translucent faces\n"
               "  G        show / hide guide lines (the true axes through the middle + drop lines to the WORLD rulers)\n"
               "  T        print the camera and the vertex table again\n"
               "  -- right pane: the FREE camera ---------------------------------------\n"
               "  W A S D  fly forward / left / back / right      Left Shift / Left Ctrl   up / down\n"
               "  arrows   look around                            , / .   field of view -5 / +5 degrees\n"
               "  V        jump the free camera to V looking at the target (then it matches the left pane)\n"
               "  B        back to the bird's-eye start position\n"
               "  O        show / hide the rays from V to every vertex\n"
               "  X        axes in the right pane: the CUBE's axes (through its centre, labels = world coordinates)\n"
               "           ->  the CAMERA's axes (dashed)  ->  both\n"
               "  ESC      quit\n"
               "(camera position, target, up vector, cube and window are in the SETUPS table at the top of main.cpp)\n";
}

// ---- drawing ---------------------------------------------------------------------------------------------------------
enum class Border { bottom, top, left, right };
constexpr bool is_horizontal(Border b) { return b == Border::bottom || b == Border::top; }

struct Axis_Info
{
  char name;
  Vec3 direction;
  Vec3 color;
};

// Tick marks, their labels (thinned so none overlap) and the name of ONE ruler on ONE border.
struct Tick { float pos; Vec2 point_px; bool major; std::string text; double priority; };

void draw_ticks_and_labels(Draw2D &d, const Frame &fr, Border border, const Vec3 &color, std::vector<Tick> ticks,
                           const std::string &name, bool guides, int label_scale = 0)
{
  const int sc = fr.text_scale;
  const int ls = (label_scale > 0) ? label_scale : sc;   // font size of the tick labels
  const float tick_len = 4.0f * static_cast<float>(sc);
  const float text_h = Draw2D::text_height(ls);
  const Vec2 lo = fr.lo, hi = fr.hi();

  // tick marks (whole numbers are longer)
  for (const Tick &t : ticks) {
    const float len = t.major ? tick_len * 1.5f : tick_len;
    switch (border) {
      case Border::bottom: d.line({ t.pos, lo.y }, { t.pos, lo.y - len }, color, 1.5f); break;
      case Border::top: d.line({ t.pos, hi.y }, { t.pos, hi.y + len }, color, 1.5f); break;
      case Border::left: d.line({ lo.x, t.pos }, { lo.x - len, t.pos }, color, 1.5f); break;
      case Border::right: d.line({ hi.x, t.pos }, { hi.x + len, t.pos }, color, 1.5f); break;
    }
    if (guides) {   // dotted drop line from the projected axis point to its tick on the border
      const Vec2 end = is_horizontal(border) ? Vec2{ t.pos, border == Border::bottom ? lo.y : hi.y }
                                             : Vec2{ border == Border::left ? lo.x : hi.x, t.pos };
      d.dashed_line(t.point_px, end, color, 1.0f, 2.0f, 4.0f, 0.35f);
    }
  }

  // labels: most important first, skipping any that would overlap one already placed
  std::vector<const Tick *> order;
  for (const Tick &t : ticks) { order.push_back(&t); }
  std::stable_sort(order.begin(), order.end(), [](const Tick *a, const Tick *b) { return a->priority < b->priority; });

  std::vector<std::pair<float, float>> placed;
  for (const Tick *t : order) {
    const float half = is_horizontal(border) ? Draw2D::text_width(t->text, ls) * 0.5f + 3.0f : text_h * 0.5f + 3.0f;
    const bool clash = std::any_of(placed.begin(), placed.end(), [&](const auto &iv) {
      return t->pos + half > iv.first && t->pos - half < iv.second;
    });
    if (clash) { continue; }
    placed.push_back({ t->pos - half, t->pos + half });

    const float gap = tick_len * 1.5f + 3.0f;
    switch (border) {
      case Border::bottom: d.text(t->text, { t->pos, lo.y - gap - text_h }, color, ls, Text_Align::center); break;
      case Border::top: d.text(t->text, { t->pos, hi.y + gap }, color, ls, Text_Align::center); break;
      case Border::left: d.text(t->text, { lo.x - gap, t->pos - text_h * 0.5f }, color, ls, Text_Align::right); break;
      case Border::right: d.text(t->text, { hi.x + gap, t->pos - text_h * 0.5f }, color, ls, Text_Align::left); break;
    }
  }

  // the ruler's name, outside the labels
  const int big = sc + 1;
  const float pad = 5.0f * static_cast<float>(sc);
  const float mid_x = (lo.x + hi.x) * 0.5f, mid_y = (lo.y + hi.y) * 0.5f;
  const float name_w = Draw2D::text_width(name, big), name_h = Draw2D::text_height(big);
  switch (border) {
    case Border::bottom: d.text(name, { mid_x, lo.y - 46.0f * sc + pad }, color, big, Text_Align::center); break;
    case Border::top: d.text(name, { mid_x, hi.y + 46.0f * sc - pad - name_h }, color, big, Text_Align::center); break;
    case Border::left: d.text(name, { lo.x - 46.0f * sc + pad + name_w * 0.5f, mid_y - name_h * 0.5f }, color, big, Text_Align::center); break;
    case Border::right: d.text(name, { hi.x + 46.0f * sc - pad - name_w * 0.5f, mid_y - name_h * 0.5f }, color, big, Text_Align::center); break;
  }
}

// WORLD ruler: ticks of ONE world axis, placed on ONE border of the frame. A tick for the value t sits
// where the world point t*direction lands on screen; on a horizontal border only its screen-x counts,
// on a vertical border only its screen-y.
void draw_world_ruler(Draw2D &d, const Frame &fr, const Mat4 &V, const Mat4 &P, const Axis_Info &axis, Border border,
                      bool guides)
{
  std::vector<Tick> ticks;
  const int n = AXIS_EXTENT_MILLI / TICK_STEP_MILLI;
  for (int k = -n; k <= n; ++k) {
    const Q value = milli(static_cast<long long>(k) * TICK_STEP_MILLI);   // exact
    const Projected p = project(V, P, axis.direction * to_float(value));
    if (!p.ok) { continue; }
    const float along = is_horizontal(border) ? p.ndc.x : p.ndc.y;
    if (std::abs(along) > 1.0f) { continue; }                          // outside the frame
    const Vec2 px = fr.to_pixels(p.ndc);
    const bool major = value.denominator() == 1;
    // whole numbers first, then by distance from 0
    ticks.push_back({ is_horizontal(border) ? px.x : px.y, px, major, fraction(value),
                      (major ? 0.0 : 1000.0) + std::abs(static_cast<double>(to_float(value))) });
  }
  draw_ticks_and_labels(d, fr, border, axis.color, std::move(ticks), std::string(1, axis.name), guides);
}

// SCREEN rulers: the 2D coordinates 0..1 themselves, a tick every 1/n, and the grid.
void draw_screen_rulers(Draw2D &d, const Frame &fr, int n, bool common_denominator_labels)
{
  const Vec2 lo = fr.lo, hi = fr.hi();
  std::vector<Tick> bottom, left;
  for (int k = 0; k <= n; ++k) {
    const float t = static_cast<float>(k) / static_cast<float>(n);
    if (k > 0 && k < n) {   // the grid, under everything else
      d.line({ lo.x + t * fr.side, lo.y }, { lo.x + t * fr.side, hi.y }, GRID_COLOR, 1.0f, 0.55f);
      d.line({ lo.x, lo.y + t * fr.side }, { hi.x, lo.y + t * fr.side }, GRID_COLOR, 1.0f, 0.55f);
    }
    const std::string text = common_denominator_labels ? over_denominator(k, n) : fraction(Q(k, n));
    const bool major = (k == 0 || k == n);
    const double priority = major ? 0.0 : 1.0 + (Q(k, n).denominator());   // simple fractions (1/2, 1/4, ...) first
    bottom.push_back({ lo.x + t * fr.side, { lo.x + t * fr.side, lo.y }, major, text, priority });
    left.push_back({ lo.y + t * fr.side, { lo.x, lo.y + t * fr.side }, major, text, priority });
  }
  // "10/12" is wider than the gap between two ticks at the normal font size: use a smaller one then
  const int ls = (common_denominator_labels && n >= 10) ? std::max(1, fr.text_scale - 1) : fr.text_scale;
  draw_ticks_and_labels(d, fr, Border::bottom, AXIS_X, std::move(bottom), "X", false, ls);
  draw_ticks_and_labels(d, fr, Border::left, AXIS_Y, std::move(left), "Y", false, ls);
}

// Which border does each world axis get? An axis that runs mostly left-right on screen gets the
// bottom (else top) border; one that runs mostly up-down gets the left (else right) border.
// Axes with the clearest direction choose first; if both borders of that kind are taken, any free one.
std::array<Border, 3> assign_borders(const Mat4 &V, const Mat4 &P, const std::array<Axis_Info, 3> &axes)
{
  struct Pref { int axis; float strength; bool horizontal; };
  std::array<Pref, 3> prefs{};
  const Projected o = project(V, P, { 0, 0, 0 });
  for (int i = 0; i < 3; ++i) {
    const Projected p = project(V, P, axes[i].direction);
    const float dx = std::abs(p.ndc.x - o.ndc.x), dy = std::abs(p.ndc.y - o.ndc.y);
    prefs[i] = { i, std::abs(dx - dy), dx >= dy };
  }
  std::sort(prefs.begin(), prefs.end(), [](const Pref &a, const Pref &b) { return a.strength > b.strength; });

  std::array<bool, 4> taken{};   // indexed by Border
  std::array<Border, 3> result{};
  for (const Pref &pr : prefs) {
    const std::array<Border, 4> order = pr.horizontal
      ? std::array<Border, 4>{ Border::bottom, Border::top, Border::left, Border::right }
      : std::array<Border, 4>{ Border::left, Border::right, Border::bottom, Border::top };
    for (Border b : order) {
      if (!taken[static_cast<int>(b)]) { taken[static_cast<int>(b)] = true; result[pr.axis] = b; break; }
    }
  }
  return result;
}

// ============================================================================
// TWO PANES. Everything below draws with the SAME functions in both: only the camera (Pane) differs.
// ============================================================================
// A Pane = one camera + the pixel rectangle its picture (NDC -1..1) is stretched over.
struct Pane
{
  Mat4 V, P;
  Vec3 eye, forward;
  bool perspective;
  Vec2 origin, size;
  int text_scale;

  Vec2 to_pixels(Vec2 ndc) const
  {
    return { origin.x + (ndc.x * 0.5f + 0.5f) * size.x, origin.y + (ndc.y * 0.5f + 0.5f) * size.y };
  }
};

Vec2 view_to_pixels(const Pane &p, const Vec3 &camera_space)
{
  const Vec4 c = p.P * Vec4{ camera_space.x, camera_space.y, camera_space.z, 1.0f };
  return p.to_pixels({ c.x / c.w, c.y / c.w });
}

// In camera space the camera looks down -Z: a point is in front of it when z < -near.
bool in_front(const Vec3 &camera_space) { return camera_space.z <= -NEAR_PLANE; }

std::optional<Vec2> pane_point(const Pane &p, const Vec3 &world)
{
  const Vec3 e = transform_point(p.V, world);
  if (!in_front(e)) { return std::nullopt; }
  return view_to_pixels(p, e);
}

// A segment, cut where it crosses the near plane (so edges do not vanish while you fly close to them).
std::optional<std::pair<Vec2, Vec2>> pane_segment(const Pane &p, const Vec3 &a, const Vec3 &b)
{
  Vec3 ea = transform_point(p.V, a), eb = transform_point(p.V, b);
  const bool ina = in_front(ea), inb = in_front(eb);
  if (!ina && !inb) { return std::nullopt; }
  const Vec3 ea0 = ea, eb0 = eb;
  if (!ina) { ea = ea0 + (eb0 - ea0) * ((-NEAR_PLANE - ea0.z) / (eb0.z - ea0.z)); }
  if (!inb) { eb = eb0 + (ea0 - eb0) * ((-NEAR_PLANE - eb0.z) / (ea0.z - eb0.z)); }
  return std::make_pair(view_to_pixels(p, ea), view_to_pixels(p, eb));
}

// A convex polygon, cut by the near plane (Sutherland-Hodgman, one plane).
std::vector<Vec2> pane_polygon(const Pane &p, const std::vector<Vec3> &world)
{
  std::vector<Vec3> e;
  for (const Vec3 &w : world) { e.push_back(transform_point(p.V, w)); }
  std::vector<Vec3> clipped;
  for (std::size_t i = 0; i < e.size(); ++i) {
    const Vec3 &cur = e[i], &nxt = e[(i + 1) % e.size()];
    const bool cin = in_front(cur), nin = in_front(nxt);
    if (cin) { clipped.push_back(cur); }
    if (cin != nin) { clipped.push_back(cur + (nxt - cur) * ((-NEAR_PLANE - cur.z) / (nxt.z - cur.z))); }
  }
  std::vector<Vec2> out;
  for (const Vec3 &c : clipped) { out.push_back(view_to_pixels(p, c)); }
  return out;
}

// The cube as seen by `pane`: translucent faces, SOLID edges that can be seen, DASHED edges that are hidden
// behind the cube, and the vertex letters (optionally with a second line of text under each letter).
void draw_cube(Draw2D &d, const Cube &cube, const Pane &pane, bool show_faces, bool show_hidden,
               const std::array<std::string, 8> *coord_text)
{
  std::array<Vec3, 8> world{};
  for (int i = 0; i < 8; ++i) { world[i] = cube.pos[i]; }

  // A face is turned towards the camera if its outward normal points at the eye
  // (perspective: eye - point on the face; orthographic: -forward). Cube = convex solid, so an
  // edge is hidden exactly when BOTH of its faces are turned away.
  std::array<bool, 6> front{};
  for (int f = 0; f < 6; ++f) {
    const Vec3 to_eye = pane.perspective ? (pane.eye - world[cube.faces[f].corner[0]]) : -pane.forward;
    front[f] = dot(cube.faces[f].normal, to_eye) > 0.0f;
  }

  if (show_faces) {
    for (int f = 0; f < 6; ++f) {
      const auto &c = cube.faces[f].corner;
      const std::vector<Vec2> poly = pane_polygon(pane, { world[c[0]], world[c[1]], world[c[2]], world[c[3]] });
      if (poly.size() >= 3) { d.polygon(poly, CUBE_FILL, front[f] ? FILL_ALPHA_FRONT : FILL_ALPHA_BACK); }
    }
  }

  for (int pass = 0; pass < 2; ++pass) {   // dashed first, solid on top
    for (const Cube::Edge &e : cube.edges) {
      const bool hidden = !front[e.face[0]] && !front[e.face[1]];
      if (hidden != (pass == 0)) { continue; }
      const auto seg = pane_segment(pane, world[e.a], world[e.b]);
      if (!seg) { continue; }
      if (hidden) {
        if (show_hidden) { d.dashed_line(seg->first, seg->second, CUBE_EDGE, 2.0f, 9.0f, 6.0f, 0.85f); }
      } else {
        d.line(seg->first, seg->second, CUBE_EDGE, 2.5f);
      }
    }
  }

  const Vec2 centre = pane.to_pixels({ 0, 0 });
  const float text_h = Draw2D::text_height(pane.text_scale);
  for (int i = 0; i < 8; ++i) {
    const auto at_px = pane_point(pane, world[i]);
    if (!at_px) { continue; }
    d.dot(*at_px, 6.0f, colors::white);
    Vec2 away = *at_px - centre;
    const float l = length(away);
    away = (l > 1e-3f) ? away * (1.0f / l) : Vec2{ 0.0f, 1.0f };
    const Vec2 at = *at_px + away * (12.0f * static_cast<float>(pane.text_scale));
    d.text(std::string(1, Cube::label(i)), { at.x, at.y - text_h * 0.5f }, colors::white, pane.text_scale, Text_Align::center);
    if (coord_text != nullptr) {
      d.text((*coord_text)[i], { at.x, at.y - text_h * 1.7f }, colors::light_grey, std::max(1, pane.text_scale - 1), Text_Align::center);
    }
  }
}

// The axes, ALWAYS drawn, with a tick and a label at every whole number from -5 to 5.
//   CUBE axes   : the frame ATTACHED TO THE CUBE: through its centre, along its edges (solid X, Y, Z). The labels are
//                 WORLD coordinates (the line y = 3/2, z = -5/2 is labelled with x), so they read like the cube's
//                 vertex coordinates. In setups 0 and 2 the cube is centred on the world origin, so this is the world
//                 frame; in setup 1 the cube's frame is parallel to the camera's but offset; in setup 2 it is rotated.
//   CAMERA axes : the camera's own axes, dashed, from its position: XC = right, YC = up, ZC = backwards (-forward).
//                 Labels are camera coordinates.
void draw_free_axes(Draw2D &d, const Pane &pane, const App &a)
{
  // value v along the axis is the point  origin + dir * (v - offset)
  struct Axis_Draw { Vec3 origin, dir; std::string name; Vec3 color; bool dashed; float offset; };
  std::vector<Axis_Draw> axes;
  if (a.axes_shown != Axes_Shown::camera) {
    const Mat4 &m = a.scene_from_world;
    const Vec3 c = transform_point(m, a.cube_center);
    axes.push_back({ c, transform_direction(m, { 1, 0, 0 }), "X", AXIS_X, false, a.cube_center.x });
    axes.push_back({ c, transform_direction(m, { 0, 1, 0 }), "Y", AXIS_Y, false, a.cube_center.y });
    axes.push_back({ c, transform_direction(m, { 0, 0, 1 }), "Z", AXIS_Z, false, a.cube_center.z });
  }
  const Basis basis = camera_basis(a.view);
  if (a.axes_shown != Axes_Shown::cube && basis.valid) {
    axes.push_back({ a.view.eye, basis.right, "XC", AXIS_X, true, 0.0f });
    axes.push_back({ a.view.eye, basis.up, "YC", AXIS_Y, true, 0.0f });
    axes.push_back({ a.view.eye, -basis.forward, "ZC", AXIS_Z, true, 0.0f });
  }

  const int sc = pane.text_scale;
  const float tick = 3.0f * static_cast<float>(sc);
  const float text_h = Draw2D::text_height(sc);
  const float ext = static_cast<float>(FREE_AXIS_EXTENT_MILLI) / static_cast<float>(SCALE);

  struct Label { Vec2 at; std::string text; Vec3 color; float priority; };
  std::vector<Label> labels;

  for (const Axis_Draw &ax : axes) {
    auto at_value = [&](float v) { return ax.origin + ax.dir * (v - ax.offset); };
    if (const auto seg = pane_segment(pane, at_value(-ext), at_value(ext))) {
      if (ax.dashed) { d.dashed_line(seg->first, seg->second, ax.color, 1.5f, 9.0f, 5.0f); }
      else { d.line(seg->first, seg->second, ax.color, 1.5f); }
    }
    if (const auto tip = pane_point(pane, at_value(ext + 0.45f))) {
      d.text(ax.name, { tip->x, tip->y - Draw2D::text_height(sc + 1) * 0.5f }, ax.color, sc + 1, Text_Align::center);
    }
    const int n = FREE_AXIS_EXTENT_MILLI / FREE_TICK_STEP_MILLI;
    for (int k = -n; k <= n; ++k) {
      const Q value = milli(static_cast<long long>(k) * FREE_TICK_STEP_MILLI);   // exact
      const float v = to_float(value);
      const auto at = pane_point(pane, at_value(v));
      const auto before = pane_point(pane, at_value(v - 0.05f)), after = pane_point(pane, at_value(v + 0.05f));
      if (!at || !before || !after) { continue; }
      Vec2 dir = *after - *before;
      const float l = length(dir);
      if (l < 1e-3f) { continue; }                       // axis seen end-on: no sensible tick direction
      dir = dir * (1.0f / l);
      Vec2 n2{ -dir.y, dir.x };                          // perpendicular on screen
      if (n2.x < -1e-3f || (std::abs(n2.x) <= 1e-3f && n2.y > 0.0f)) { n2 = n2 * -1.0f; }   // labels always on the same side
      d.line(*at - n2 * tick, *at + n2 * tick, ax.color, 1.5f);
      labels.push_back({ *at + n2 * (tick + 5.0f * static_cast<float>(sc)), fraction(value),
                         k == 0 ? colors::white : ax.color, static_cast<float>(std::abs(k)) + (ax.dashed ? 0.5f : 0.0f) });
    }
  }

  // labels, nearest to the origin first; skip any that would land on one already drawn
  std::stable_sort(labels.begin(), labels.end(), [](const Label &a, const Label &b) { return a.priority < b.priority; });
  std::vector<Vec2> used;
  const float min_gap = text_h * 1.7f;
  for (const Label &lb : labels) {
    if (lb.at.x < pane.origin.x || lb.at.x > pane.origin.x + pane.size.x || lb.at.y < pane.origin.y ||
        lb.at.y > pane.origin.y + pane.size.y) { continue; }
    const bool clash = std::any_of(used.begin(), used.end(), [&](const Vec2 &u) { return length(u - lb.at) < min_gap; });
    if (clash) { continue; }
    used.push_back(lb.at);
    d.text(lb.text, { lb.at.x, lb.at.y - text_h * 0.5f }, lb.color, sc, Text_Align::center);
  }
}

// The LEFT camera drawn inside the free view: its position V, the pyramid of what it sees (up to the far side
// of the cube), a dashed line to its target, and (key O) rays from V to every vertex.
void draw_setup_camera(Draw2D &d, const Pane &pane, const App &a)
{
  const Basis b = camera_basis(a.view);
  if (!b.valid) { return; }
  const Setup &s = a.current();
  const Vec3 eye = a.view.eye;

  float wl, wr, wb, wt;   // the window on the image plane at distance 1
  if (s.use_fov) {
    const float t = std::tan(radians(FOV_DEG) * 0.5f);
    wl = -t; wr = t; wb = -t; wt = t;
  } else {
    wl = to_float(s.window.left); wr = to_float(s.window.right); wb = to_float(s.window.bottom); wt = to_float(s.window.top);
  }

  float far_depth = 1.0f;
  for (const Vec3 &v : a.cube.pos) { far_depth = std::max(far_depth, dot(b.forward, v - eye)); }
  far_depth += 0.3f;
  const float dist = a.ortho_distance();

  // corner (wx, wy) of the window, `z` in front of the camera. Perspective: the window grows with z.
  // Orthographic: the window keeps its size (that of the target's distance).
  auto corner = [&](float z, float wx, float wy) {
    const float k = a.view.perspective ? z : dist;
    return eye + b.forward * z + b.right * (wx * k) + b.up * (wy * k);
  };
  const std::array<std::array<float, 2>, 4> win{ { { wl, wb }, { wr, wb }, { wr, wt }, { wl, wt } } };

  auto line3 = [&](const Vec3 &p, const Vec3 &q, float alpha, float thickness) {
    if (const auto seg = pane_segment(pane, p, q)) { d.line(seg->first, seg->second, CAMERA_GIZMO, thickness, alpha); }
  };
  std::array<Vec3, 4> far_c{};
  for (int i = 0; i < 4; ++i) { far_c[i] = corner(far_depth, win[i][0], win[i][1]); }
  for (int i = 0; i < 4; ++i) {
    line3(far_c[i], far_c[(i + 1) % 4], 0.8f, 1.5f);                               // the far rectangle
    if (a.view.perspective) {
      line3(eye, far_c[i], 0.8f, 1.5f);                                           // V to its corners
    } else {
      const Vec3 near_c = corner(0.0f, win[i][0], win[i][1]);                     // box: near rectangle ...
      line3(near_c, far_c[i], 0.8f, 1.5f);
      line3(near_c, corner(0.0f, win[(i + 1) % 4][0], win[(i + 1) % 4][1]), 0.8f, 1.5f);
    }
  }

  if (a.show_rays) {
    for (const Vec3 &v : a.cube.pos) {
      if (const auto seg = pane_segment(pane, eye, v)) {
        d.dashed_line(seg->first, seg->second, colors::light_blue, 1.0f, 4.0f, 4.0f, 0.55f);
      }
    }
  }

  if (const auto seg = pane_segment(pane, eye, a.view.target)) {
    d.dashed_line(seg->first, seg->second, CAMERA_GIZMO, 1.0f, 3.0f, 5.0f, 0.5f);   // line of sight to the target
  }
  if (const auto at = pane_point(pane, eye)) {
    d.dot(*at, 9.0f, CAMERA_GIZMO);
    d.text("V", { at->x + 9.0f, at->y + 5.0f }, CAMERA_GIZMO, pane.text_scale + 1);
  }
}

void draw_left_pane(App &a, const Rect &rect)
{
  const Frame fr = make_frame(rect);
  const Mat4 V = view_matrix(a.view);
  const Mat4 P = projection_matrix(a.view, a.current(), a.ortho_distance());
  const Basis basis = camera_basis(a.view);
  const auto exact = exact_screen_coords(a);
  Draw2D &d = a.draw;

  // ---- grid (SCREEN rulers) goes under the cube: draw it first ---------------------------------------------
  int ticks_n = FALLBACK_DENOMINATOR;
  if (exact) {
    const long long lcd = common_denominator(*exact);
    ticks_n = static_cast<int>(lcd <= MAX_COMMON_DENOMINATOR ? lcd : 12);
  }
  if (a.ruler_mode == Ruler_Mode::screen) {
    draw_screen_rulers(d, fr, ticks_n, a.common_denominator_labels);
  }

  // ---- the cube (faces, dashed / solid edges, letters) -----------------------------------------------------------
  const Pane pane{ V, P, a.view.eye, basis.forward, a.view.perspective, fr.lo, { fr.side, fr.side }, fr.text_scale };
  std::array<std::string, 8> coord_text;
  if (a.show_coords) {
    for (int i = 0; i < 8; ++i) {
      const Projected p = project(V, P, a.cube.pos[i]);
      coord_text[i] = exact ? "(" + fraction((*exact)[i][0]) + "," + fraction((*exact)[i][1]) + ")"
                            : "(" + decimal(p.ndc.x * 0.5f + 0.5f, 2) + "," + decimal(p.ndc.y * 0.5f + 0.5f, 2) + ")";
    }
  }
  draw_cube(d, a.cube, pane, a.show_faces, a.show_hidden, a.show_coords ? &coord_text : nullptr);

  // ---- guide lines: the real axes through the middle (only on request) -----------------------------------------------------
  const std::array<Axis_Info, 3> axes{ Axis_Info{ 'X', { 1, 0, 0 }, AXIS_X },
                                       Axis_Info{ 'Y', { 0, 1, 0 }, AXIS_Y },
                                       Axis_Info{ 'Z', { 0, 0, 1 }, AXIS_Z } };
  if (a.show_guides) {
    const float ext = static_cast<float>(AXIS_EXTENT_MILLI) / static_cast<float>(SCALE);
    for (const Axis_Info &ax : axes) {
      const Projected p0 = project(V, P, ax.direction * -ext), p1 = project(V, P, ax.direction * ext);
      if (p0.ok && p1.ok) { d.line(fr.to_pixels(p0.ndc), fr.to_pixels(p1.ndc), ax.color, 1.0f, 0.45f); }
    }
  }

  // ---- the frame (what the camera sees) and, in WORLD mode, the rulers on its borders -------------------------------------------
  d.rect_outline(fr.lo, fr.hi(), FRAME_COLOR, 2.0f);
  if (a.ruler_mode == Ruler_Mode::world) {
    const std::array<Border, 3> borders = assign_borders(V, P, axes);
    for (int i = 0; i < 3; ++i) { draw_world_ruler(d, fr, V, P, axes[i], borders[i], a.show_guides); }
  }

  // ---- little heads-up text, top-left corner ---------------------------------------------------------------------------------------
  const int sc = fr.text_scale;
  const float h = Draw2D::text_height(sc);
  const float top = rect.y + rect.h - 10.0f;
  d.text(a.current().name, { rect.x + 10.0f, top - h }, colors::white, sc);
  d.text(std::string(a.view.perspective ? "PERSPECTIVE" : "ORTHOGRAPHIC") + "  UP " + decimal(a.view.up, 2),
         { rect.x + 10.0f, top - 2.2f * h }, colors::light_grey, sc);
  d.text(a.ruler_mode == Ruler_Mode::screen
           ? "SCREEN RULERS  STEP 1/" + std::to_string(ticks_n) + (exact ? " (EXACT)" : " (DECIMAL)")
           : "WORLD RULERS",
         { rect.x + 10.0f, top - 3.4f * h }, colors::light_grey, sc);
  if (!basis.valid) {
    d.text("UP PARALLEL TO VIEW DIRECTION", { rect.x + 10.0f, top - 4.6f * h }, colors::crimson, sc);
  }

}

void draw_right_pane(App &a, const Rect &rect)
{
  Draw2D &d = a.draw;
  const int sc = std::max(1, static_cast<int>(std::min(rect.w, rect.h)) / 400);
  const Pane pane{ a.free_cam.view(), a.free_cam.projection(rect.w / rect.h), a.free_cam.position, a.free_cam.front(),
                   true, { rect.x, rect.y }, { rect.w, rect.h }, sc };

  d.polygon({ { rect.x, rect.y }, { rect.x + rect.w, rect.y }, { rect.x + rect.w, rect.y + rect.h }, { rect.x, rect.y + rect.h } },
            FREE_BACKGROUND, 1.0f);
  draw_free_axes(d, pane, a);
  draw_setup_camera(d, pane, a);
  draw_cube(d, a.cube, pane, a.show_faces, a.show_hidden, nullptr);

  const float h = Draw2D::text_height(sc);
  const float top = rect.y + rect.h - 10.0f;
  d.text("FREE CAMERA", { rect.x + 10.0f, top - h }, colors::white, sc);
  d.text("POS " + decimal(a.free_cam.position, 1), { rect.x + 10.0f, top - 2.2f * h }, colors::light_grey, sc);
  d.text("FOV " + decimal(a.free_cam.fov_y_deg, 0) + "  (V: GO TO V)", { rect.x + 10.0f, top - 3.4f * h }, colors::light_grey, sc);
  d.text(std::string("AXES (X): ") + (a.axes_shown == Axes_Shown::cube ? "CUBE (THROUGH ITS CENTER)" : a.axes_shown == Axes_Shown::camera ? "CAMERA (DASHED)" : "CUBE + CAMERA (DASHED)"),
         { rect.x + 10.0f, top - 4.6f * h }, colors::light_grey, sc);
  d.line({ rect.x, rect.y }, { rect.x, rect.y + rect.h }, FRAME_COLOR, 2.0f);   // the divider
}

void display(GLFWwindow *window)
{
  App &a = *app;
  int fb_w{}, fb_h{};
  glfwGetFramebufferSize(window, &fb_w, &fb_h);
  glViewport(0, 0, fb_w, fb_h);
  glClearColor(BACKGROUND.x, BACKGROUND.y, BACKGROUND.z, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  if (fb_w < 2 || fb_h <= 0) { return; }

  const int half = fb_w / 2;
  const Rect left{ 0.0f, 0.0f, static_cast<float>(half), static_cast<float>(fb_h) };
  const Rect right{ static_cast<float>(half), 0.0f, static_cast<float>(fb_w - half), static_cast<float>(fb_h) };

  glEnable(GL_SCISSOR_TEST);   // each pane draws only inside its own half of the window
  glScissor(0, 0, half, fb_h);
  a.draw.begin(fb_w, fb_h);
  draw_left_pane(a, left);
  a.draw.end();

  glScissor(half, 0, fb_w - half, fb_h);
  a.draw.begin(fb_w, fb_h);
  draw_right_pane(a, right);
  a.draw.end();
  glDisable(GL_SCISSOR_TEST);
}

// ---- keyboard: callback for one-shot events -------------------------------------------------------------------------------------------
// Registered in main() with glfwSetKeyCallback(); GLFW calls it from inside glfwPollEvents().
void try_set_up(App &a, const Vec3 &new_up, const char *why, const std::optional<Q3> &exact_up = std::nullopt)
{
  View candidate = a.view;
  candidate.up = new_up;
  if (!camera_basis(candidate).valid) {
    std::cout << "\n[ignored] " << why << ": up vector " << decimal(new_up)
              << " is parallel to the view direction, so f x up = 0 and look-at is undefined.\n";
    return;
  }
  a.view.up = new_up;
  a.up_exact = exact_up.has_value();
  if (exact_up) { a.up_q = *exact_up; }
  std::cout << "\n" << why << " -> up vector = " << decimal(a.view.up);
  print_camera(a);
  print_table(a);
}

void choose_setup(App &a, int index)
{
  a.select_setup(index);
  std::cout << "\n==== key " << (index == 2 ? 0 : 8 + index) << " ====\n";
  print_setup(a);
  print_camera(a);
  print_table(a);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (app == nullptr || action == GLFW_RELEASE) { return; }
  App &a = *app;
  const bool pressed = (action == GLFW_PRESS);
  const Q one = 1, zero = 0;

  switch (key) {
    case GLFW_KEY_ESCAPE: if (pressed) { glfwSetWindowShouldClose(window, GLFW_TRUE); } break;
    case GLFW_KEY_8: if (pressed) { choose_setup(a, 0); } break;
    case GLFW_KEY_9: if (pressed) { choose_setup(a, 1); } break;
    case GLFW_KEY_0: if (pressed) { choose_setup(a, 2); } break;
    case GLFW_KEY_X:
      if (pressed) {
        a.axes_shown = a.axes_shown == Axes_Shown::cube ? Axes_Shown::camera
                       : a.axes_shown == Axes_Shown::camera ? Axes_Shown::both : Axes_Shown::cube;
      }
      break;
    case GLFW_KEY_P:
      if (pressed) { a.view.perspective = !a.view.perspective; print_camera(a); print_table(a); }
      break;
    case GLFW_KEY_M:
      if (pressed) { a.ruler_mode = (a.ruler_mode == Ruler_Mode::screen) ? Ruler_Mode::world : Ruler_Mode::screen; }
      break;
    case GLFW_KEY_L: if (pressed) { a.show_coords = !a.show_coords; } break;
    case GLFW_KEY_C: if (pressed) { a.common_denominator_labels = !a.common_denominator_labels; } break;
    case GLFW_KEY_O: if (pressed) { a.show_rays = !a.show_rays; } break;
    case GLFW_KEY_B: if (pressed) { a.reset_free_camera(); std::cout << "\nfree camera: back to the bird's-eye position\n"; } break;
    case GLFW_KEY_COMMA:
    case GLFW_KEY_PERIOD:   // also while held
      a.free_cam.fov_y_deg = std::clamp(a.free_cam.fov_y_deg + (key == GLFW_KEY_PERIOD ? 5.0f : -5.0f), 10.0f, 120.0f);
      break;
    case GLFW_KEY_V:
      if (pressed) {
        // the free camera takes the position of V and looks at the target; its field of view is chosen so the
        // left camera's window fits (a symmetric view cannot show an off-axis window exactly: it is a crop)
        const Setup &st = a.current();
        aim_camera(a.free_cam, a.view.eye, a.view.target);
        float tan_half = std::tan(radians(FOV_DEG) * 0.5f);
        if (!st.use_fov) {
          tan_half = std::max({ std::abs(to_float(st.window.left)), std::abs(to_float(st.window.right)),
                                std::abs(to_float(st.window.bottom)), std::abs(to_float(st.window.top)) });
        }
        a.free_cam.fov_y_deg = degrees(2.0f * std::atan(tan_half));
        std::cout << "\nfree camera: moved to V = " << decimal(a.view.eye) << " looking at " << decimal(a.view.target)
                  << ", fov " << decimal(a.free_cam.fov_y_deg, 1) << " deg\n";
        if (!st.use_fov) {
          std::cout << "  (this setup's window is off-axis, so the free camera's symmetric view shows it as the part\n"
                       "   of its picture from the centre to the top-right corner)\n";
        }
        if (a.view.up.x != 0.0f || a.view.up.z != 0.0f || a.view.up.y <= 0.0f) {
          std::cout << "  (the free camera cannot roll: it always keeps +Y up, so this up vector " << decimal(a.view.up)
                    << " is not reproduced: the free picture is the upright one)\n";
        }
      }
      break;
    case GLFW_KEY_1: if (pressed) { try_set_up(a, { 1, 0, 0 }, "key 1", Q3{ one, zero, zero }); } break;
    case GLFW_KEY_2: if (pressed) { try_set_up(a, { -1, 0, 0 }, "key 2", Q3{ -one, zero, zero }); } break;
    case GLFW_KEY_3: if (pressed) { try_set_up(a, { 0, 1, 0 }, "key 3", Q3{ zero, one, zero }); } break;
    case GLFW_KEY_4: if (pressed) { try_set_up(a, { 0, -1, 0 }, "key 4", Q3{ zero, -one, zero }); } break;
    case GLFW_KEY_5: if (pressed) { try_set_up(a, { 0, 0, 1 }, "key 5", Q3{ zero, zero, one }); } break;
    case GLFW_KEY_6: if (pressed) { try_set_up(a, { 0, 0, -1 }, "key 6", Q3{ zero, zero, -one }); } break;
    case GLFW_KEY_Q:
    case GLFW_KEY_E: {   // also while held (GLFW_REPEAT)
      const Basis b = camera_basis(a.view);
      if (!b.valid) { break; }
      const float deg = (key == GLFW_KEY_E) ? 5.0f : -5.0f;
      try_set_up(a, transform_direction(mat4_rotate(radians(deg), b.forward), a.view.up), key == GLFW_KEY_E ? "roll +5" : "roll -5");
      break;
    }
    case GLFW_KEY_R:
      if (pressed) {
        a.select_setup(a.setup);// back to the setup's own camera, up vector and PERSPECTIVE
        std::cout << "\nreset";
        print_camera(a);
        print_table(a);
      }
      break;
    case GLFW_KEY_H: if (pressed) { a.show_hidden = !a.show_hidden; } break;
    case GLFW_KEY_F: if (pressed) { a.show_faces = !a.show_faces; } break;
    case GLFW_KEY_G: if (pressed) { a.show_guides = !a.show_guides; } break;
    case GLFW_KEY_T: if (pressed) { print_camera(a); print_table(a); } break;
    default: break;
  }
}

int main()
{
  if (glfwInit() == 0) {
    cerr << "Failed to initialize GLFW" << endl;
    return -1;
  }

  request_opengl_version();// 4.3 core profile
  glfwWindowHint(GLFW_SAMPLES, 4);// smoother diagonal lines

  GLFWwindow *window = glfwCreateWindow({ .width = 1600, .height = 800, .title = "Cube from the camera (left) and a free camera (right)" });
  if (window == nullptr) {
    cerr << "Failed to create window" << endl;
    glfwTerminate();
    return -1;
  }
  glfwSetWindowAspectRatio(window, 2, 1);// two square panes side by side
  glfwMakeContextCurrent(window);
  if (gladLoadGL(glfwGetProcAddress) == 0) { exit(EXIT_FAILURE); }
  glfwSetKeyCallback(window, key_callback);
  glfwSwapInterval(1);

  app = std::make_unique<App>(START_SETUP);

  // ---- printed once at start ---------------------------------------------------------------------------
  std::cout << "============ pedagogic_visualizations_look_at ============\n";
  print_setup(*app);
  if (!camera_basis(app->view).valid) {
    std::cout << "\nWARNING: the up vector is parallel to the view direction (or the camera is ON the target):\n"
                 "look-at is undefined. Press 1..6 to choose another up vector.\n";
  }
  print_camera(*app);
  print_table(*app);
  print_keys();
  std::cout.flush();

  double previous_time = glfwGetTime();
  while (not glfwWindowShouldClose(window)) {
    const double now = glfwGetTime();
    const float dt = std::min(0.1f, static_cast<float>(now - previous_time));
    previous_time = now;
    app->free_cam.process_keyboard(window, dt);// W A S D, Left Shift / Left Ctrl, arrows: polled every frame (held keys)

    display(window);
    glfwSwapBuffers(window);
    glfwPollEvents();// key_callback() is invoked from HERE
  }

  app.reset();// GPU objects before the window
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
