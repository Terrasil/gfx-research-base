#include <gfx/research/screenshot.hpp>

#include <algorithm>
#include <filesystem>
#include <vector>

#include <glad/gl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace gfx::research {
    bool save_framebuffer_png(const std::filesystem::path& path, int width, int height, int x, int y) {
        if (width <= 0 || height <= 0) return false;
        if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());

        std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        const std::size_t row_bytes = static_cast<std::size_t>(width) * 4;
        std::vector<unsigned char> flipped(pixels.size());
        for (int row = 0; row < height; ++row) {
            const auto* src = pixels.data() + static_cast<std::size_t>(height - 1 - row) * row_bytes;
            auto* dst = flipped.data() + static_cast<std::size_t>(row) * row_bytes;
            std::copy_n(src, row_bytes, dst);
        }
        return stbi_write_png(path.string().c_str(), width, height, 4, flipped.data(), width * 4) != 0;
    }
}
