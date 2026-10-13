#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

// ============================================================================
// Camera.hpp — a small fly-through camera.
//
// The camera owns its state (position + yaw/pitch + lens settings) and answers
// the two matrices a 3D program needs every frame:
//
//     Mat4 view        = camera.view();                 // world -> camera space
//     Mat4 projection  = camera.projection(aspect);     // camera -> clip space
//
// Typical frame:
//     camera.process_keyboard(window, dt);              // optional movement
//     Mat4 mvp = camera.projection(aspect) * camera.view() * model;
//     program.set_uniform("u_mvp", mvp);
//
// Angles are stored in DEGREES (easier to reason about when tuning by hand)
// and converted to radians only where the math needs them.
// ============================================================================

#include <GLFW/glfw3.h>

#include "Mat4.hpp"

class Camera
{
public:
  // ---- pose ----------------------------------------------------------------
  Vec3 position{ 0.0f, 0.0f, 3.0f };
  Vec3 world_up{ 0.0f, 1.0f, 0.0f };
  float yaw_deg{ -90.0f };// -90 => looking down -Z (OpenGL convention)
  float pitch_deg{ 0.0f };// + looks up, clamped to +/-MAX_PITCH_DEG (Mat4.hpp) to avoid gimbal lock

  // ---- lens ----------------------------------------------------------------
  float fov_y_deg{ 45.0f };
  float near_plane{ 0.1f };
  float far_plane{ 100.0f };

  // ---- tuning knobs for process_keyboard() ---------------------------------
  float move_speed{ 2.5f };   // units per second
  float turn_speed_deg{ 60.0f };// degrees per second

  static constexpr float DEG_TO_RAD = 0.017453292519943295f;

  // Unit vector the camera is looking along, derived from yaw/pitch.
  Vec3 front() const
  {
    const float yaw = yaw_deg * DEG_TO_RAD;
    const float pitch = pitch_deg * DEG_TO_RAD;
    return normalize(Vec3{ std::cos(pitch) * std::cos(yaw),
                           std::sin(pitch),
                           std::cos(pitch) * std::sin(yaw) });
  }

  // Unit vector pointing to the camera's right (for strafing).
  Vec3 right() const { return normalize(cross(front(), world_up)); }

  // world -> camera space
  Mat4 view() const { return mat4_look_at(position, position + front(), world_up); }

  // camera -> clip space. `aspect` = framebuffer width / height.
  Mat4 projection(float aspect) const
  {
    return mat4_perspective(fov_y_deg * DEG_TO_RAD, aspect, near_plane, far_plane);
  }

  // Optional built-in controls (call once per frame with the frame time):
  //   W / S        forward / backward
  //   A / D        strafe left / right
  //   Left Shift   up   (along world_up)
  //   Left Ctrl    down
  //   arrow keys   look around (yaw / pitch)
  // Replace or ignore this if you want mouse-look or your own key mapping —
  // it is a convenience, not part of the camera's contract.
  void process_keyboard(GLFWwindow *window, float dt)
  {
    const float step = move_speed * dt;
    const float turn = turn_speed_deg * dt;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { position = position + front() * step; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { position = position - front() * step; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { position = position - right() * step; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { position = position + right() * step; }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) { position = position + world_up * step; }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) { position = position - world_up * step; }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { yaw_deg -= turn; }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { yaw_deg += turn; }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) { pitch_deg += turn; }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) { pitch_deg -= turn; }

    // Gimbal-lock guard: keep the pitch just shy of straight up/down, where
    // front() would become parallel to world_up and right() would collapse.
    if (pitch_deg > MAX_PITCH_DEG) { pitch_deg = MAX_PITCH_DEG; }
    if (pitch_deg < -MAX_PITCH_DEG) { pitch_deg = -MAX_PITCH_DEG; }
  }
};

#endif// __CAMERA_HPP__
