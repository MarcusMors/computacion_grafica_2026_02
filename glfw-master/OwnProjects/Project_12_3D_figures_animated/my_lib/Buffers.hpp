#ifndef __BUFFERS_HPP__
#define __BUFFERS_HPP__

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

// ============================================================================
// RAII wrappers for the everyday OpenGL vertex objects.
//
//   VAO (Vertex_Array)  - state container: attribute layout + which VBO feeds
//                         each attribute + which EBO is bound. Holds NO data.
//   VBO (Buffer)        - raw GPU memory with vertex data (GL_ARRAY_BUFFER).
//   EBO (Index_Buffer)  - raw GPU memory with indices (GL_ELEMENT_ARRAY_BUFFER).
//                         Templated on the index type so the GL enum
//                         (GL_UNSIGNED_BYTE/SHORT/INT) is DEDUCED, never
//                         memorized by the programmer.
//
// These classes guarantee every created object is destroyed, even when an
// exception is thrown halfway through init().
// ============================================================================

// ----------------------------------------------------------------------------
// Small float vector types for vertex structs.
// Use them as members of your own Vertex struct; the attribute() template
// deduces the component count and GL type from them.
// (Homogeneous coordinates are fine: Vec4{ x, y, z, w } with w as a float.)
// ----------------------------------------------------------------------------
struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };

// Maps a C++ member type to (component count, GL element type).
// Supported: float, Vec2, Vec3, Vec4, std::array<float, N>.
namespace detail {
template<typename T>
struct Attribute_Info;// deliberately undefined: unsupported member type

template<> struct Attribute_Info<float> { static constexpr GLint components = 1; static constexpr GLenum gl_type = GL_FLOAT; };
template<> struct Attribute_Info<Vec2>  { static constexpr GLint components = 2; static constexpr GLenum gl_type = GL_FLOAT; };
template<> struct Attribute_Info<Vec3>  { static constexpr GLint components = 3; static constexpr GLenum gl_type = GL_FLOAT; };
template<> struct Attribute_Info<Vec4>  { static constexpr GLint components = 4; static constexpr GLenum gl_type = GL_FLOAT; };
template<std::size_t N>
struct Attribute_Info<std::array<float, N>> { static constexpr GLint components = static_cast<GLint>(N); static constexpr GLenum gl_type = GL_FLOAT; };

// offsetof() for member pointers (&Vertex::field). Standard-layout required.
template<typename Vertex, typename Member>
inline std::size_t offset_of(Member Vertex::*member)
{
  static_assert(std::is_standard_layout_v<Vertex>,
                "Vertex struct must be standard-layout (no virtuals, no base classes)");
  return reinterpret_cast<std::size_t>(&(reinterpret_cast<Vertex *>(0)->*member));
}
}// namespace detail

// Generic GPU buffer (used as GL_ARRAY_BUFFER in this project).
class Buffer
{
public:
  GLuint id{};

  Buffer() { glGenBuffers(1, &id); }
  ~Buffer() { glDeleteBuffers(1, &id); }

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  void bind(GLenum target = GL_ARRAY_BUFFER) const { glBindBuffer(target, id); }

  // Upload raw bytes. usage: GL_STATIC_DRAW (rarely changes) or
  // GL_DYNAMIC_DRAW (rewritten often, e.g. every frame).
  void upload(const void *data, GLsizeiptr bytes, GLenum usage = GL_STATIC_DRAW,
              GLenum target = GL_ARRAY_BUFFER) const
  {
    glBindBuffer(target, id);
    glBufferData(target, bytes, data, usage);
  }

  // Convenience: upload a whole std::vector.
  template<typename T>
  void upload(const std::vector<T> &v, GLenum usage = GL_STATIC_DRAW,
              GLenum target = GL_ARRAY_BUFFER) const
  {
    upload(v.data(), static_cast<GLsizeiptr>(v.size() * sizeof(T)), usage, target);
  }
};

// ----------------------------------------------------------------------------
// Index buffer (EBO/IBO). The template parameter IS the index type, so
// glDrawElements' "type" argument is deduced automatically:
//   Index_Buffer<uint8_t>  -> GL_UNSIGNED_BYTE   (max index 255)
//   Index_Buffer<uint16_t> -> GL_UNSIGNED_SHORT  (max index 65 535)
//   Index_Buffer<uint32_t> -> GL_UNSIGNED_INT    (max index ~4.29e9)
// ----------------------------------------------------------------------------
template<typename IndexT>
class Index_Buffer
{
  static_assert(std::is_same_v<IndexT, uint8_t> ||
                std::is_same_v<IndexT, uint16_t> ||
                std::is_same_v<IndexT, uint32_t>,
                "Index_Buffer only supports uint8_t, uint16_t or uint32_t "
                "(the only legal types for glDrawElements)");

public:
  GLuint id{};

  static constexpr GLenum  gl_type = std::is_same_v<IndexT, uint8_t>  ? GL_UNSIGNED_BYTE
                                     : std::is_same_v<IndexT, uint16_t> ? GL_UNSIGNED_SHORT
                                                                        : GL_UNSIGNED_INT;
  static constexpr GLsizei index_size = static_cast<GLsizei>(sizeof(IndexT));

  Index_Buffer() { glGenBuffers(1, &id); }
  ~Index_Buffer() { glDeleteBuffers(1, &id); }

  Index_Buffer(const Index_Buffer &) = delete;
  Index_Buffer &operator=(const Index_Buffer &) = delete;

  // NOTE: prefer Vertex_Array::attach_indices() — binding an EBO while a VAO
  // is bound records the EBO inside THAT VAO; calling bind()/upload() directly
  // records it into whatever VAO happens to be bound at the time.
  void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id); }

  void upload(const IndexT *indices, std::size_t count, GLenum usage = GL_STATIC_DRAW) const
  {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(count * sizeof(IndexT)),
                 indices, usage);
  }
  void upload(const std::vector<IndexT> &indices, GLenum usage = GL_STATIC_DRAW) const
  {
    upload(indices.data(), indices.size(), usage);
  }
};

// ----------------------------------------------------------------------------
// Vertex Array Object (VAO): remembers the attribute layout + bound buffers.
// ----------------------------------------------------------------------------
class Vertex_Array
{
public:
  GLuint id{};

  Vertex_Array() { glGenVertexArrays(1, &id); }
  ~Vertex_Array() { glDeleteVertexArrays(1, &id); }

  Vertex_Array(const Vertex_Array &) = delete;
  Vertex_Array &operator=(const Vertex_Array &) = delete;

  void bind() const { glBindVertexArray(id); }
  static void unbind() { glBindVertexArray(0); }

  // ---- low-level: explicit numbers (mirrors the book) ---------------------
  // Describe one float attribute, e.g. position (vec2) at offset 0:
  //     vao.attribute(0, 2, sizeof(Vertex), offsetof(Vertex, x));
  void attribute(GLuint index, GLint components, GLsizei stride_bytes, std::size_t offset_bytes) const
  {
    glVertexAttribPointer(index, components, GL_FLOAT, GL_FALSE,
                          stride_bytes, reinterpret_cast<void *>(offset_bytes));
    glEnableVertexAttribArray(index);
  }

  // ---- Option C: layout deduced from your vertex struct -------------------
  //     struct Vertex { Vec2 pos; Vec3 color; };
  //     vao.attribute(0, &Vertex::pos);    // -> location 0, 2 floats, offset 0
  //     vao.attribute(1, &Vertex::color);  // -> location 1, 3 floats, offset 8
  // Component count, GL type, stride and offset are all deduced from the
  // member pointer. "location" connects to layout(location = N) in the
  // vertex shader.
  template<typename Vertex, typename Member>
  void attribute(GLuint location, Member Vertex::*member) const
  {
    using Info = detail::Attribute_Info<std::remove_cv_t<Member>>;
    glVertexAttribPointer(location, Info::components, Info::gl_type, GL_FALSE,
                          static_cast<GLsizei>(sizeof(Vertex)),
                          reinterpret_cast<void *>(detail::offset_of(member)));
    glEnableVertexAttribArray(location);
  }

  // Bind this VAO and upload `vertices` into `vbo` (records nothing by itself;
  // the following attribute() calls capture the still-bound vbo).
  template<typename T>
  void attach_vertices(const Buffer &vbo, const std::vector<T> &vertices,
                       GLenum usage = GL_STATIC_DRAW) const
  {
    bind();
    vbo.upload(vertices, usage);// upload() binds GL_ARRAY_BUFFER itself
  }

  // Bind this VAO, upload `indices` into `ebo`, and RECORD the EBO inside this
  // VAO (the GL_ELEMENT_ARRAY_BUFFER binding is part of VAO state).
  // Afterwards draw_elements() knows the index type and count automatically.
  template<typename IndexT>
  void attach_indices(const Index_Buffer<IndexT> &ebo, const IndexT *indices, std::size_t count,
                      GLenum usage = GL_STATIC_DRAW)
  {
    bind();
    ebo.upload(indices, count, usage);// binds the EBO while THIS VAO is bound
    index_type_  = Index_Buffer<IndexT>::gl_type;
    index_count_ = static_cast<GLsizei>(count);
    index_size_  = Index_Buffer<IndexT>::index_size;
  }
  template<typename IndexT>
  void attach_indices(const Index_Buffer<IndexT> &ebo, const std::vector<IndexT> &indices,
                      GLenum usage = GL_STATIC_DRAW)
  {
    attach_indices(ebo, indices.data(), indices.size(), usage);
  }
  // Attach an already-uploaded EBO without touching its data.
  template<typename IndexT>
  void attach_indices(const Index_Buffer<IndexT> &ebo, GLsizei count)
  {
    bind();
    ebo.bind();
    index_type_  = Index_Buffer<IndexT>::gl_type;
    index_count_ = count;
    index_size_  = Index_Buffer<IndexT>::index_size;
  }

  // ---- drawing (which array is used is evident: the one this object is) ---
  // Draw ALL attached indices:            vao.draw_elements(GL_TRIANGLES);
  // Draw a range (indices, not bytes):    vao.draw_elements(GL_TRIANGLES, 3, 3);
  void draw_elements(GLenum mode) const { draw_elements(mode, index_count_, 0); }
  void draw_elements(GLenum mode, GLsizei count, GLintptr first_index = 0) const
  {
    if (index_type_ == 0) {
      throw std::logic_error(
          "Vertex_Array::draw_elements: no Index_Buffer attached "
          "(call attach_indices() first, or use draw_arrays())");
    }
    glBindVertexArray(id);
    glDrawElements(mode, count, index_type_,
                   reinterpret_cast<const void *>(
                       static_cast<std::uintptr_t>(first_index) * index_size_));
  }

  // Non-indexed drawing (vertices in storage order, no EBO needed):
  //     vao.draw_arrays(GL_TRIANGLES, 3);        // first 3 vertices
  //     vao.draw_arrays(GL_TRIANGLES, 3, 2);     // vertices 2..4
  void draw_arrays(GLenum mode, GLsizei count, GLint first_vertex = 0) const
  {
    glBindVertexArray(id);
    glDrawArrays(mode, first_vertex, count);
  }

private:
  GLenum  index_type_{};  // 0 = no index buffer attached
  GLsizei index_count_{};
  GLsizei index_size_{};
};

#endif// __BUFFERS_HPP__
