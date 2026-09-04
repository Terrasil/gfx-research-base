#include <gfx/research/assets.hpp>

#include <string>

namespace gfx::research {
    const std::filesystem::path& asset_root() {
        static const std::filesystem::path root = GFX_RESEARCH_BASE_ASSET_DIR;
        return root;
    }

    std::filesystem::path asset_path(const std::string_view relativePath) {
        return asset_root() / std::string(relativePath);
    }

    std::filesystem::path model_path(const ModelAsset model) {
        switch (model) {
            case ModelAsset::Bunny: return asset_root() / "models/bunny.obj";
            case ModelAsset::Dragon: return asset_root() / "models/dragon.obj";
            case ModelAsset::Elephant: return asset_root() / "models/elephant.obj";
            case ModelAsset::Sponza: return asset_root() / "models/sponza.obj";
            case ModelAsset::Suzanne: return asset_root() / "models/suzanne.obj";
            case ModelAsset::Teapot: return asset_root() / "models/teapot.obj";
        }
        return {};
    }
}
