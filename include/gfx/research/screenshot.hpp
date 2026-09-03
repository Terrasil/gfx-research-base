#pragma once

#include <filesystem>

namespace gfx::research {
    bool save_framebuffer_png(const std::filesystem::path& path, int width, int height, int x = 0, int y = 0);
}
