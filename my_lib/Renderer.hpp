#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

// ============================================================================
// Renderer.hpp — draws meshes: flat-colored faces + an optional outline.
//
//     Renderer renderer;                               // in init() (needs the GL context)
//     ...
//     renderer.begin_frame(window, camera);            // viewport, clear, camera matrices
//     renderer.draw(gpu_mesh, model_matrix);           // or: figure.draw(renderer);
//
// THE OUTLINE (how it works, so you can debug it)
//   1. faces are drawn with GL_TRIANGLES, pushed slightly BACK in depth with
//      glPolygonOffset. Without that, a line lying exactly on a face fights
//      with it for the same depth value ("z-fighting") and half of it vanishes.
//   2. edges are drawn with GL_LINES in one uniform color (default white),
//      using the SAME vertex buffer and the SAME model matrix.
//   Hidden edges stay hidden because both passes share the depth buffer.
// ============================================================================

#include <GLFW/glfw3.h>

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Buffers.hpp"
#include "Camera.hpp"
#include "Mat4.hpp"
#include "Mesh.hpp"
#include "_cpp.hpp"// Program

// ----------------------------------------------------------------------------
// Gpu_Mesh — a Mesh living in GPU memory (VAO + VBO + one EBO that holds the
// triangle indices followed by the edge indices). Non-copyable (owns GL
// objects): share it with std::shared_ptr<const Gpu_Mesh> — Mesh_Figure does.
// ----------------------------------------------------------------------------
class Gpu_Mesh
{
public:
  explicit Gpu_Mesh(const Mesh &mesh)
    : centroid_{ mesh.centroid() }, bounds_{ mesh.bounds() },
      triangle_index_count_{ static_cast<GLsizei>(mesh.triangles.size()) },
      edge_index_count_{ static_cast<GLsizei>(mesh.edges.size()) }
  {
    // One index buffer: [ triangle indices | edge indices ].
    std::vector<std::uint32_t> indices = mesh.triangles;
    indices.insert(indices.end(), mesh.edges.begin(), mesh.edges.end());

    vao_.attach_vertices(vbo_, mesh.vertices);
    vao_.attribute(0, &Mesh_Vertex::pos);  // layout(location = 0) in vertex shader
    vao_.attribute(1, &Mesh_Vertex::color);// layout(location = 1)
    vao_.attach_indices(ebo_, indices);
  }

  void draw_faces() const { vao_.draw_elements(GL_Draw::TRIANGLES, triangle_index_count_, 0); }
  void draw_edges() const { vao_.draw_elements(GL_Draw::LINES, edge_index_count_, triangle_index_count_); }

  const Vec3 &centroid() const { return centroid_; }
  const Aabb &bounds() const { return bounds_; }

private:
  Vertex_Array vao_;
  Buffer vbo_;
  Index_Buffer<std::uint32_t> ebo_;
  Vec3 centroid_;
  Aabb bounds_;
  GLsizei triangle_index_count_;
  GLsizei edge_index_count_;
};

// ----------------------------------------------------------------------------
// Renderer
// ----------------------------------------------------------------------------
class Renderer
{
public:
  // ---- settings you can change at any time ---------------------------------
  bool outline{ true };           // draw the edges of every figure
  Vec3 outline_color{ colors::white };
  float line_width{ 2.0f };

  Renderer(const Renderer &) = delete;// owns GL programs
  Renderer &operator=(const Renderer &) = delete;

  Renderer()
  {
    fill_.attach({ .vertex = Shader_Source(std::string_view{ vertex_source }),
                   .fragment = Shader_Source(std::string_view{ fill_fragment_source }) });
    line_.attach({ .vertex = Shader_Source(std::string_view{ vertex_source }),
                   .fragment = Shader_Source(std::string_view{ line_fragment_source }) });
  }

  // Once per frame, before drawing: viewport, clear, camera, GL state.
  void begin_frame(GLFWwindow *window, const Camera &camera, const Vec3 &clear_color = colors::black)
  {
    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    const float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;

    glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    begin(camera.projection(aspect) * camera.view());
  }

  // Lower-level: set the view-projection matrix yourself (no clear, no viewport).
  void begin(const Mat4 &view_projection)
  {
    view_projection_ = view_projection;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);// pushes FACES back so the outline wins the depth test
    glPolygonOffset(1.0f, 1.0f);
    glLineWidth(line_width);
  }

  // Draw one mesh with the given model matrix (local space -> world space).
  // outline_override: color for THIS mesh's outline (nullopt = renderer's color).
  void draw(const Gpu_Mesh &mesh, const Mat4 &model, bool with_outline = true,
            std::optional<Vec3> outline_override = std::nullopt) const
  {
    const Mat4 mvp = view_projection_ * model;

    fill_.set_uniform("u_mvp", mvp);// set_uniform() also binds the program
    mesh.draw_faces();

    if (outline && with_outline) {
      line_.set_uniform("u_mvp", mvp);
      line_.set_uniform("u_color", outline_override.value_or(outline_color));
      mesh.draw_edges();
    }
  }

private:
  static constexpr std::string_view vertex_source = R"(#version 430 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
uniform mat4 u_mvp;
out vec3 v_color;
void main(void)
{
  v_color = a_color;
  gl_Position = u_mvp * vec4(a_pos, 1.0);
}
)";

  static constexpr std::string_view fill_fragment_source = R"(#version 430 core
in vec3 v_color;
out vec4 color;
void main(void)
{
  color = vec4(v_color, 1.0);
}
)";

  static constexpr std::string_view line_fragment_source = R"(#version 430 core
uniform vec3 u_color;
out vec4 color;
void main(void)
{
  color = vec4(u_color, 1.0);
}
)";

  Program fill_;
  Program line_;
  Mat4 view_projection_{ Mat4::identity() };
};

#endif// __RENDERER_HPP__
