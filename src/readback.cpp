#include <gfx/research/readback.hpp>

#include <algorithm>
#include <cstddef>

#include <glad/gl.h>

namespace gfx::research {
    std::vector<float> read_texture_rgba(unsigned int texture, int width, int height) {
        if (!texture || width <= 0 || height <= 0) return {};
        std::vector<float> pixels(static_cast<std::size_t>(width) * height * 4);
        glGetTextureImage(texture, 0, GL_RGBA, GL_FLOAT, static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());
        return pixels;
    }

    std::vector<float> read_texture_depth(unsigned int texture, int width, int height) {
        if (!texture || width <= 0 || height <= 0) return {};
        std::vector<float> pixels(static_cast<std::size_t>(width) * height);
        glGetTextureImage(texture, 0, GL_DEPTH_COMPONENT, GL_FLOAT, static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());
        return pixels;
    }

    std::array<float, 4> read_texture_pixel_rgba(unsigned int texture, int x, int y) {
        std::array<float, 4> pixel{};
        if (!texture) return pixel;
        glGetTextureSubImage(texture, 0, std::max(x, 0), std::max(y, 0), 0, 1, 1, 1, GL_RGBA, GL_FLOAT,
            static_cast<GLsizei>(sizeof(pixel)), pixel.data());
        return pixel;
    }

    float read_texture_pixel_depth(unsigned int texture, int x, int y) {
        float depth = 1.0f;
        if (!texture) return depth;
        glGetTextureSubImage(texture, 0, std::max(x, 0), std::max(y, 0), 0, 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
            static_cast<GLsizei>(sizeof(depth)), &depth);
        return depth;
    }

    std::vector<float> read_framebuffer_rgba(int x, int y, int width, int height) {
        if (width <= 0 || height <= 0) return {};
        std::vector<float> pixels(static_cast<std::size_t>(width) * height * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(x, y, width, height, GL_RGBA, GL_FLOAT, pixels.data());
        return pixels;
    }

    std::array<float, 4> read_framebuffer_pixel_rgba(int x, int y) {
        std::array<float, 4> pixel{};
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, pixel.data());
        return pixel;
    }
}
