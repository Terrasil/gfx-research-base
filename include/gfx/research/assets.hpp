#pragma once

#include <filesystem>
#include <string_view>

namespace gfx::research {
    enum class ModelAsset {
        Bunny,
        Dragon,
        Elephant,
        Sponza,
        Suzanne,
        Teapot
    };

    [[nodiscard]] const std::filesystem::path& asset_root();
    [[nodiscard]] std::filesystem::path asset_path(std::string_view relativePath);
    [[nodiscard]] std::filesystem::path model_path(ModelAsset model);
}
