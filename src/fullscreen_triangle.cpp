#include <gfx/research/fullscreen_triangle.hpp>

#include <utility>

#include <glad/gl.h>

namespace gfx::research {
    FullscreenTriangle::FullscreenTriangle() = default;
    FullscreenTriangle::FullscreenTriangle(FullscreenTriangle&& other) noexcept : vao_(std::exchange(other.vao_, 0)) {}
    FullscreenTriangle& FullscreenTriangle::operator=(FullscreenTriangle&& other) noexcept {
        if (this == &other) return *this;
        if (vao_) glDeleteVertexArrays(1, &vao_);
        vao_ = std::exchange(other.vao_, 0);
        return *this;
    }
    FullscreenTriangle::~FullscreenTriangle() { if (vao_) glDeleteVertexArrays(1, &vao_); }
    void FullscreenTriangle::draw() const {
        if (!vao_) glCreateVertexArrays(1, &vao_);
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}
