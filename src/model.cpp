#include <gfx/research/model.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

namespace gfx::research {
    namespace {
        const unsigned char* accessor_data(const tinygltf::Model& model, const tinygltf::Accessor& accessor, std::size_t& stride) {
            if (accessor.bufferView < 0 || static_cast<std::size_t>(accessor.bufferView) >= model.bufferViews.size()) return nullptr;
            const auto& view = model.bufferViews[static_cast<std::size_t>(accessor.bufferView)];
            if (view.buffer < 0 || static_cast<std::size_t>(view.buffer) >= model.buffers.size()) return nullptr;
            const auto& buffer = model.buffers[static_cast<std::size_t>(view.buffer)];
            const int byte_stride = accessor.ByteStride(view);
            if (byte_stride < 0) return nullptr;
            stride = byte_stride > 0
                ? static_cast<std::size_t>(byte_stride)
                : static_cast<std::size_t>(tinygltf::GetComponentSizeInBytes(accessor.componentType) * tinygltf::GetNumComponentsInType(accessor.type));
            const std::size_t offset = view.byteOffset + accessor.byteOffset;
            return offset < buffer.data.size() ? buffer.data.data() + offset : nullptr;
        }

        glm::mat4 node_local_transform(const tinygltf::Node& node) {
            if (node.matrix.size() == 16) {
                glm::mat4 result(1.0f);
                for (int column = 0; column < 4; ++column) {
                    for (int row = 0; row < 4; ++row) {
                        result[column][row] = static_cast<float>(node.matrix[static_cast<std::size_t>(column * 4 + row)]);
                    }
                }
                return result;
            }

            glm::mat4 result(1.0f);
            if (node.translation.size() == 3) {
                result = glm::translate(result, glm::vec3(
                    static_cast<float>(node.translation[0]),
                    static_cast<float>(node.translation[1]),
                    static_cast<float>(node.translation[2])
                ));
            }
            if (node.rotation.size() == 4) {
                const glm::quat rotation(
                    static_cast<float>(node.rotation[3]),
                    static_cast<float>(node.rotation[0]),
                    static_cast<float>(node.rotation[1]),
                    static_cast<float>(node.rotation[2])
                );
                result *= glm::mat4_cast(rotation);
            }
            if (node.scale.size() == 3) {
                result = glm::scale(result, glm::vec3(
                    static_cast<float>(node.scale[0]),
                    static_cast<float>(node.scale[1]),
                    static_cast<float>(node.scale[2])
                ));
            }
            return result;
        }

        std::uint32_t read_index(const unsigned char* data, int componentType) {
            switch (componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return *reinterpret_cast<const std::uint8_t*>(data);
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return *reinterpret_cast<const std::uint16_t*>(data);
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return *reinterpret_cast<const std::uint32_t*>(data);
                default: return std::numeric_limits<std::uint32_t>::max();
            }
        }
    }

    void Model::clear() {
        mesh_.clear();
        vertices_.clear();
        indices_.clear();
        bounds_min_ = {};
        bounds_max_ = {};
    }

    bool Model::load(const std::filesystem::path& path) {
        clear();
        std::string ext = path.extension().string();
        std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const bool loaded = ext == ".obj" ? load_obj(path) : (ext == ".gltf" || ext == ".glb") ? load_gltf(path) : false;
        if (!loaded) return false;
        finalize();
        return true;
    }

    bool Model::load_obj(const std::filesystem::path& path) {
        tinyobj::ObjReaderConfig config;
        config.triangulate = true;
        config.vertex_color = false;

        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(path.string(), config)) {
            std::fprintf(stderr, "OBJ: %s\n", reader.Error().c_str());
            return false;
        }
        if (!reader.Warning().empty()) std::fprintf(stderr, "OBJ warning: %s\n", reader.Warning().c_str());

        const auto& attrib = reader.GetAttrib();
        for (const auto& shape : reader.GetShapes()) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                if (index.vertex_index >= 0) {
                    const auto i = static_cast<std::size_t>(index.vertex_index) * 3;
                    if (i + 2 < attrib.vertices.size()) std::memcpy(&vertex.position, &attrib.vertices[i], sizeof(float) * 3);
                }
                if (index.normal_index >= 0) {
                    const auto i = static_cast<std::size_t>(index.normal_index) * 3;
                    if (i + 2 < attrib.normals.size()) std::memcpy(&vertex.normal, &attrib.normals[i], sizeof(float) * 3);
                }
                if (index.texcoord_index >= 0) {
                    const auto i = static_cast<std::size_t>(index.texcoord_index) * 2;
                    if (i + 1 < attrib.texcoords.size()) {
                        vertex.uv.x = attrib.texcoords[i];
                        vertex.uv.y = attrib.texcoords[i + 1];
                    }
                }
                indices_.push_back(static_cast<std::uint32_t>(vertices_.size()));
                vertices_.push_back(vertex);
            }
        }
        return !vertices_.empty();
    }

    bool Model::load_gltf(const std::filesystem::path& path) {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string error;
        std::string warning;
        std::string ext = path.extension().string();
        std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const bool loaded = ext == ".glb"
            ? loader.LoadBinaryFromFile(&model, &error, &warning, path.string())
            : loader.LoadASCIIFromFile(&model, &error, &warning, path.string());

        if (!warning.empty()) std::fprintf(stderr, "glTF warning: %s\n", warning.c_str());
        if (!loaded) {
            std::fprintf(stderr, "glTF: %s\n", error.c_str());
            return false;
        }

        const auto append_mesh = [&](int mesh_index, const glm::mat4& transform) {
            if (mesh_index < 0 || static_cast<std::size_t>(mesh_index) >= model.meshes.size()) return;
            const glm::mat3 normal_transform = glm::inverseTranspose(glm::mat3(transform));
            const auto& mesh = model.meshes[static_cast<std::size_t>(mesh_index)];

            for (const auto& primitive : mesh.primitives) {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1) continue;
                const auto position_it = primitive.attributes.find("POSITION");
                if (position_it == primitive.attributes.end()) continue;

                const auto& positions = model.accessors[static_cast<std::size_t>(position_it->second)];
                if (positions.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || positions.type != TINYGLTF_TYPE_VEC3) continue;

                const tinygltf::Accessor* normals = nullptr;
                const tinygltf::Accessor* uvs = nullptr;
                if (auto it = primitive.attributes.find("NORMAL"); it != primitive.attributes.end()) normals = &model.accessors[static_cast<std::size_t>(it->second)];
                if (auto it = primitive.attributes.find("TEXCOORD_0"); it != primitive.attributes.end()) uvs = &model.accessors[static_cast<std::size_t>(it->second)];

                std::size_t position_stride = 0;
                std::size_t normal_stride = 0;
                std::size_t uv_stride = 0;
                const auto* position_data = accessor_data(model, positions, position_stride);
                const auto* normal_data = normals ? accessor_data(model, *normals, normal_stride) : nullptr;
                const auto* uv_data = uvs ? accessor_data(model, *uvs, uv_stride) : nullptr;
                if (!position_data) continue;

                const std::uint32_t base = static_cast<std::uint32_t>(vertices_.size());
                for (std::size_t i = 0; i < positions.count; ++i) {
                    Vertex vertex{};
                    glm::vec3 position{};
                    std::memcpy(&position, position_data + i * position_stride, sizeof(float) * 3);
                    vertex.position = glm::vec3(transform * glm::vec4(position, 1.0f));

                    if (normal_data && normals->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && normals->type == TINYGLTF_TYPE_VEC3) {
                        glm::vec3 normal{};
                        std::memcpy(&normal, normal_data + i * normal_stride, sizeof(float) * 3);
                        vertex.normal = glm::normalize(normal_transform * normal);
                    }
                    if (uv_data && uvs->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && uvs->type == TINYGLTF_TYPE_VEC2) {
                        std::memcpy(&vertex.uv, uv_data + i * uv_stride, sizeof(float) * 2);
                    }
                    vertices_.push_back(vertex);
                }

                if (primitive.indices >= 0) {
                    const auto& accessor = model.accessors[static_cast<std::size_t>(primitive.indices)];
                    std::size_t stride = 0;
                    const auto* data = accessor_data(model, accessor, stride);
                    if (!data) continue;
                    for (std::size_t i = 0; i < accessor.count; ++i) {
                        const std::uint32_t index = read_index(data + i * stride, accessor.componentType);
                        if (index == std::numeric_limits<std::uint32_t>::max()) return;
                        indices_.push_back(base + index);
                    }
                } else {
                    for (std::uint32_t i = 0; i < positions.count; ++i) indices_.push_back(base + i);
                }
            }
        };

        const auto traverse = [&](auto&& self, int node_index, const glm::mat4& parent) -> void {
            if (node_index < 0 || static_cast<std::size_t>(node_index) >= model.nodes.size()) return;
            const auto& node = model.nodes[static_cast<std::size_t>(node_index)];
            const glm::mat4 transform = parent * node_local_transform(node);
            if (node.mesh >= 0) append_mesh(node.mesh, transform);
            for (const int child : node.children) self(self, child, transform);
        };

        if (!model.scenes.empty()) {
            int scene_index = model.defaultScene >= 0 ? model.defaultScene : 0;
            scene_index = std::clamp(scene_index, 0, static_cast<int>(model.scenes.size()) - 1);
            for (const int node : model.scenes[static_cast<std::size_t>(scene_index)].nodes) traverse(traverse, node, glm::mat4(1.0f));
        } else if (!model.nodes.empty()) {
            std::vector<bool> child(model.nodes.size(), false);
            for (const auto& node : model.nodes) for (const int index : node.children) if (index >= 0 && static_cast<std::size_t>(index) < child.size()) child[static_cast<std::size_t>(index)] = true;
            for (std::size_t i = 0; i < model.nodes.size(); ++i) if (!child[i]) traverse(traverse, static_cast<int>(i), glm::mat4(1.0f));
        } else {
            for (std::size_t i = 0; i < model.meshes.size(); ++i) append_mesh(static_cast<int>(i), glm::mat4(1.0f));
        }

        return !vertices_.empty();
    }

    void Model::finalize() {
        recalculate_missing_normals();
        recalculate_bounds();
        mesh_.upload(vertices_, indices_);
    }

    void Model::recalculate_missing_normals() {
        bool missing = false;
        for (const auto& vertex : vertices_) {
            if (glm::dot(vertex.normal, vertex.normal) < 1e-12f) {
                missing = true;
                break;
            }
        }
        if (!missing || indices_.size() < 3) return;

        for (auto& vertex : vertices_) vertex.normal = glm::vec3(0.0f);
        for (std::size_t i = 0; i + 2 < indices_.size(); i += 3) {
            Vertex& a = vertices_[indices_[i]];
            Vertex& b = vertices_[indices_[i + 1]];
            Vertex& c = vertices_[indices_[i + 2]];
            const glm::vec3 normal = glm::cross(b.position - a.position, c.position - a.position);
            a.normal += normal;
            b.normal += normal;
            c.normal += normal;
        }
        for (auto& vertex : vertices_) {
            const float length_squared = glm::dot(vertex.normal, vertex.normal);
            vertex.normal = length_squared > 1e-12f ? glm::normalize(vertex.normal) : glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    void Model::recalculate_bounds() {
        if (vertices_.empty()) {
            bounds_min_ = bounds_max_ = {};
            return;
        }
        bounds_min_ = glm::vec3(std::numeric_limits<float>::max());
        bounds_max_ = glm::vec3(std::numeric_limits<float>::lowest());
        for (const auto& vertex : vertices_) {
            bounds_min_ = glm::min(bounds_min_, vertex.position);
            bounds_max_ = glm::max(bounds_max_, vertex.position);
        }
    }
}
