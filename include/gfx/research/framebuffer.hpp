#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace gfx::research {
    struct ColorAttachmentDesc {
        unsigned int internal_format = 0x881Au; // GL_RGBA16F
        unsigned int min_filter = 0x2601u;      // GL_LINEAR
        unsigned int mag_filter = 0x2601u;      // GL_LINEAR
    };

    class Framebuffer {
    public:
        Framebuffer() = default;
        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;
        Framebuffer(Framebuffer&& other) noexcept;
        Framebuffer& operator=(Framebuffer&& other) noexcept;
        ~Framebuffer();

        bool create(int width, int height, std::span<const ColorAttachmentDesc> colors, bool depth = true, unsigned int depth_format = 0x8CACu);
        bool resize(int width, int height);
        void clear();
        void bind() const;
        static void bind_default();

        [[nodiscard]] unsigned int id() const { return fbo_; }
        [[nodiscard]] unsigned int color(std::size_t index = 0) const { return index < colors_.size() ? colors_[index] : 0; }
        [[nodiscard]] unsigned int depth() const { return depth_; }
        [[nodiscard]] int width() const { return width_; }
        [[nodiscard]] int height() const { return height_; }

    private:
        bool allocate();

        unsigned int fbo_ = 0;
        std::vector<unsigned int> colors_;
        unsigned int depth_ = 0;
        int width_ = 0;
        int height_ = 0;
        bool has_depth_ = true;
        unsigned int depth_format_ = 0x8CACu;
        std::vector<ColorAttachmentDesc> color_descs_;
    };
}
