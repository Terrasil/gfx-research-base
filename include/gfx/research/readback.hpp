#pragma once

#include <array>
#include <vector>

namespace gfx::research {
    std::vector<float> read_texture_rgba(unsigned int texture, int width, int height);
    std::vector<float> read_texture_depth(unsigned int texture, int width, int height);
    std::array<float, 4> read_texture_pixel_rgba(unsigned int texture, int x, int y);
    float read_texture_pixel_depth(unsigned int texture, int x, int y);
    std::vector<float> read_framebuffer_rgba(int x, int y, int width, int height);
    std::array<float, 4> read_framebuffer_pixel_rgba(int x, int y);
}
