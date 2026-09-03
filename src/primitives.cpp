#include <gfx/research/primitives.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

namespace gfx::research {
    Mesh make_plane(float size, std::uint32_t subdivisions) {
        subdivisions = std::max(subdivisions, 1u);
        const std::uint32_t count = subdivisions + 1;
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        vertices.reserve(static_cast<std::size_t>(count) * count);
        indices.reserve(static_cast<std::size_t>(subdivisions) * subdivisions * 6);

        for (std::uint32_t z = 0; z < count; ++z) {
            for (std::uint32_t x = 0; x < count; ++x) {
                const float u = static_cast<float>(x) / subdivisions;
                const float v = static_cast<float>(z) / subdivisions;
                vertices.push_back({
                    glm::vec3((u - 0.5f) * size, 0.0f, (v - 0.5f) * size),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec2(u, v)
                });
            }
        }

        for (std::uint32_t z = 0; z < subdivisions; ++z) {
            for (std::uint32_t x = 0; x < subdivisions; ++x) {
                const std::uint32_t a = z * count + x;
                const std::uint32_t b = a + 1;
                const std::uint32_t c = a + count;
                const std::uint32_t d = c + 1;
                indices.insert(indices.end(), {a, c, b, b, c, d});
            }
        }
        return Mesh(vertices, indices);
    }

    Mesh make_grid(float size, std::uint32_t resolution) {
        return make_plane(size, std::max(resolution, 1u));
    }

    Mesh make_cube(float size) {
        const float h = size * 0.5f;
        const std::vector<Vertex> vertices = {
            {{-h,-h, h},{0,0,1},{0,0}}, {{ h,-h, h},{0,0,1},{1,0}}, {{ h, h, h},{0,0,1},{1,1}}, {{-h, h, h},{0,0,1},{0,1}},
            {{ h,-h,-h},{0,0,-1},{0,0}}, {{-h,-h,-h},{0,0,-1},{1,0}}, {{-h, h,-h},{0,0,-1},{1,1}}, {{ h, h,-h},{0,0,-1},{0,1}},
            {{-h,-h,-h},{-1,0,0},{0,0}}, {{-h,-h, h},{-1,0,0},{1,0}}, {{-h, h, h},{-1,0,0},{1,1}}, {{-h, h,-h},{-1,0,0},{0,1}},
            {{ h,-h, h},{1,0,0},{0,0}}, {{ h,-h,-h},{1,0,0},{1,0}}, {{ h, h,-h},{1,0,0},{1,1}}, {{ h, h, h},{1,0,0},{0,1}},
            {{-h, h, h},{0,1,0},{0,0}}, {{ h, h, h},{0,1,0},{1,0}}, {{ h, h,-h},{0,1,0},{1,1}}, {{-h, h,-h},{0,1,0},{0,1}},
            {{-h,-h,-h},{0,-1,0},{0,0}}, {{ h,-h,-h},{0,-1,0},{1,0}}, {{ h,-h, h},{0,-1,0},{1,1}}, {{-h,-h, h},{0,-1,0},{0,1}}
        };
        std::vector<std::uint32_t> indices;
        indices.reserve(36);
        for (std::uint32_t face = 0; face < 6; ++face) {
            const std::uint32_t o = face * 4;
            indices.insert(indices.end(), {o, o + 1, o + 2, o, o + 2, o + 3});
        }
        return Mesh(vertices, indices);
    }

    Mesh make_uv_sphere(float radius, std::uint32_t segments, std::uint32_t rings) {
        segments = std::max(segments, 3u);
        rings = std::max(rings, 2u);
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;

        for (std::uint32_t y = 0; y <= rings; ++y) {
            const float v = static_cast<float>(y) / rings;
            const float phi = v * std::numbers::pi_v<float>;
            for (std::uint32_t x = 0; x <= segments; ++x) {
                const float u = static_cast<float>(x) / segments;
                const float theta = u * std::numbers::pi_v<float> * 2.0f;
                const glm::vec3 normal(
                    std::sin(phi) * std::cos(theta),
                    std::cos(phi),
                    std::sin(phi) * std::sin(theta)
                );
                vertices.push_back({normal * radius, normal, glm::vec2(u, v)});
            }
        }

        const std::uint32_t stride = segments + 1;
        for (std::uint32_t y = 0; y < rings; ++y) {
            for (std::uint32_t x = 0; x < segments; ++x) {
                const std::uint32_t a = y * stride + x;
                const std::uint32_t b = a + 1;
                const std::uint32_t c = a + stride;
                const std::uint32_t d = c + 1;
                indices.insert(indices.end(), {a, c, b, b, c, d});
            }
        }
        return Mesh(vertices, indices);
    }
}
