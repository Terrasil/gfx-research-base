#include <gfx/research/mesh.hpp>

#include <cstddef>
#include <utility>

#include <glad/gl.h>

namespace gfx::research {
    Mesh::Mesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices, unsigned int primitive) {
        upload(vertices, indices, primitive);
    }

    Mesh::Mesh(Mesh&& other) noexcept { *this = std::move(other); }

    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this == &other) return *this;
        clear();
        vao_ = std::exchange(other.vao_, 0);
        vbo_ = std::exchange(other.vbo_, 0);
        ebo_ = std::exchange(other.ebo_, 0);
        primitive_ = other.primitive_;
        vertex_count_ = std::exchange(other.vertex_count_, 0);
        index_count_ = std::exchange(other.index_count_, 0);
        return *this;
    }

    Mesh::~Mesh() { clear(); }

    void Mesh::upload(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices, unsigned int primitive) {
        clear();
        if (vertices.empty()) return;

        primitive_ = primitive;
        vertex_count_ = vertices.size();
        index_count_ = indices.size();

        glCreateVertexArrays(1, &vao_);
        glCreateBuffers(1, &vbo_);
        glNamedBufferStorage(vbo_, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), 0);
        glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex));

        glEnableVertexArrayAttrib(vao_, 0);
        glEnableVertexArrayAttrib(vao_, 1);
        glEnableVertexArrayAttrib(vao_, 2);
        glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribFormat(vao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(vao_, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
        glVertexArrayAttribBinding(vao_, 0, 0);
        glVertexArrayAttribBinding(vao_, 1, 0);
        glVertexArrayAttribBinding(vao_, 2, 0);

        if (!indices.empty()) {
            glCreateBuffers(1, &ebo_);
            glNamedBufferStorage(ebo_, static_cast<GLsizeiptr>(indices.size_bytes()), indices.data(), 0);
            glVertexArrayElementBuffer(vao_, ebo_);
        }
    }

    void Mesh::clear() {
        if (ebo_) glDeleteBuffers(1, &ebo_);
        if (vbo_) glDeleteBuffers(1, &vbo_);
        if (vao_) glDeleteVertexArrays(1, &vao_);
        vao_ = vbo_ = ebo_ = 0;
        vertex_count_ = 0;
        index_count_ = 0;
    }

    void Mesh::draw() const {
        if (!vao_) return;
        glBindVertexArray(vao_);
        if (index_count_) glDrawElements(primitive_, static_cast<GLsizei>(index_count_), GL_UNSIGNED_INT, nullptr);
        else glDrawArrays(primitive_, 0, static_cast<GLsizei>(vertex_count_));
    }

    void Mesh::draw_instanced(int instances) const {
        if (!vao_ || instances <= 0) return;
        glBindVertexArray(vao_);
        if (index_count_) glDrawElementsInstanced(primitive_, static_cast<GLsizei>(index_count_), GL_UNSIGNED_INT, nullptr, instances);
        else glDrawArraysInstanced(primitive_, 0, static_cast<GLsizei>(vertex_count_), instances);
    }
}
