#include <gfx/research/framebuffer.hpp>

#include <cstdio>
#include <utility>

#include <glad/gl.h>

namespace gfx::research {
    Framebuffer::Framebuffer(Framebuffer&& other) noexcept { *this = std::move(other); }

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
        if (this == &other) return *this;
        clear();
        fbo_ = std::exchange(other.fbo_, 0);
        colors_ = std::move(other.colors_);
        depth_ = std::exchange(other.depth_, 0);
        width_ = std::exchange(other.width_, 0);
        height_ = std::exchange(other.height_, 0);
        has_depth_ = other.has_depth_;
        depth_format_ = other.depth_format_;
        color_descs_ = std::move(other.color_descs_);
        return *this;
    }

    Framebuffer::~Framebuffer() { clear(); }

    bool Framebuffer::create(int width, int height, std::span<const ColorAttachmentDesc> colors, bool depth, unsigned int depth_format) {
        clear();
        if (width <= 0 || height <= 0 || colors.empty()) return false;
        width_ = width;
        height_ = height;
        has_depth_ = depth;
        depth_format_ = depth_format;
        color_descs_.assign(colors.begin(), colors.end());
        return allocate();
    }

    bool Framebuffer::resize(int width, int height) {
        if (width <= 0 || height <= 0) return false;
        if (width == width_ && height == height_ && fbo_) return true;
        if (color_descs_.empty()) return false;
        if (depth_) glDeleteTextures(1, &depth_);
        if (!colors_.empty()) glDeleteTextures(static_cast<GLsizei>(colors_.size()), colors_.data());
        if (fbo_) glDeleteFramebuffers(1, &fbo_);
        depth_ = 0;
        colors_.clear();
        fbo_ = 0;
        width_ = width;
        height_ = height;
        return allocate();
    }

    bool Framebuffer::allocate() {
        glCreateFramebuffers(1, &fbo_);
        colors_.resize(color_descs_.size());
        glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(colors_.size()), colors_.data());

        std::vector<GLenum> draw_buffers;
        draw_buffers.reserve(colors_.size());
        for (std::size_t i = 0; i < colors_.size(); ++i) {
            const auto& desc = color_descs_[i];
            glTextureStorage2D(colors_[i], 1, desc.internal_format, width_, height_);
            glTextureParameteri(colors_[i], GL_TEXTURE_MIN_FILTER, static_cast<GLint>(desc.min_filter));
            glTextureParameteri(colors_[i], GL_TEXTURE_MAG_FILTER, static_cast<GLint>(desc.mag_filter));
            glTextureParameteri(colors_[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(colors_[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            const GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
            glNamedFramebufferTexture(fbo_, attachment, colors_[i], 0);
            draw_buffers.push_back(attachment);
        }
        glNamedFramebufferDrawBuffers(fbo_, static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data());

        if (has_depth_) {
            glCreateTextures(GL_TEXTURE_2D, 1, &depth_);
            glTextureStorage2D(depth_, 1, depth_format_, width_, height_);
            glTextureParameteri(depth_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(depth_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(depth_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(depth_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            const GLenum attachment = depth_format_ == GL_DEPTH24_STENCIL8 || depth_format_ == GL_DEPTH32F_STENCIL8
                ? GL_DEPTH_STENCIL_ATTACHMENT
                : GL_DEPTH_ATTACHMENT;
            glNamedFramebufferTexture(fbo_, attachment, depth_, 0);
        }

        const GLenum status = glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::fprintf(stderr, "Framebuffer incomplete: 0x%x\n", status);
            return false;
        }
        return true;
    }

    void Framebuffer::clear() {
        if (depth_) glDeleteTextures(1, &depth_);
        if (!colors_.empty()) glDeleteTextures(static_cast<GLsizei>(colors_.size()), colors_.data());
        if (fbo_) glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
        depth_ = 0;
        colors_.clear();
        width_ = height_ = 0;
    }

    void Framebuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, fbo_); }
    void Framebuffer::bind_default() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
}
