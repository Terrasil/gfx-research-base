#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

#include <glm/vec3.hpp>

#include <gfx/research/mesh.hpp>

namespace gfx::research {
    class Model {
    public:
        Model() = default;
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        Model(Model&&) noexcept = default;
        Model& operator=(Model&&) noexcept = default;

        bool load(const std::filesystem::path& path);
        void draw() const { mesh_.draw(); }
        void clear();

        [[nodiscard]] std::size_t vertex_count() const { return vertices_.size(); }
        [[nodiscard]] std::size_t index_count() const { return indices_.size(); }
        [[nodiscard]] const glm::vec3& bounds_min() const { return bounds_min_; }
        [[nodiscard]] const glm::vec3& bounds_max() const { return bounds_max_; }
        [[nodiscard]] const Mesh& gpu_mesh() const { return mesh_; }

    private:
        bool load_obj(const std::filesystem::path& path);
        bool load_gltf(const std::filesystem::path& path);
        void finalize();
        void recalculate_missing_normals();
        void recalculate_bounds();

        std::vector<Vertex> vertices_;
        std::vector<std::uint32_t> indices_;
        Mesh mesh_;
        glm::vec3 bounds_min_{};
        glm::vec3 bounds_max_{};
    };
}
