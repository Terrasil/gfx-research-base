#pragma once

#include <cstdint>

#include <gfx/research/mesh.hpp>

namespace gfx::research {
    Mesh make_plane(float size = 2.0f, std::uint32_t subdivisions = 1);
    Mesh make_grid(float size = 2.0f, std::uint32_t resolution = 64);
    Mesh make_cube(float size = 1.0f);
    Mesh make_uv_sphere(float radius = 1.0f, std::uint32_t segments = 48, std::uint32_t rings = 24);
}
