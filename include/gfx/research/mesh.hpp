#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace gfx::research {
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 normal{};
        glm::vec2 uv{};
    };

    class Mesh {
    public:
        Mesh() = default;
        Mesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices, unsigned int primitive = 0x0004u);
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        ~Mesh();

        void upload(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices, unsigned int primitive = 0x0004u);
        void clear();
        void draw() const;
        void draw_instanced(int instances) const;

        [[nodiscard]] std::size_t vertex_count() const { return vertex_count_; }
        [[nodiscard]] std::size_t index_count() const { return index_count_; }
        [[nodiscard]] unsigned int vao() const { return vao_; }

    private:
        unsigned int vao_ = 0;
        unsigned int vbo_ = 0;
        unsigned int ebo_ = 0;
        unsigned int primitive_ = 0x0004u;
        std::size_t vertex_count_ = 0;
        std::size_t index_count_ = 0;
    };
}
