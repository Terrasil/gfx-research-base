#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gfx::research {
    enum class ShaderStage {
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment,
        Compute
    };

    struct ShaderFile {
        ShaderStage stage;
        std::filesystem::path path;
    };

    struct ShaderSource {
        ShaderStage stage;
        std::string source;
    };

    class Shader {
    public:
        Shader() = default;
        Shader(std::span<const ShaderFile> files);
        Shader(const char* vertex_source, const char* fragment_source);
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;
        ~Shader();

        bool load(std::span<const ShaderFile> files);
        bool load(std::span<const ShaderSource> sources);
        bool reload();
        void clear();
        void bind() const;

        void set_bool(const char* name, bool value) const;
        void set_int(const char* name, int value) const;
        void set_uint(const char* name, unsigned int value) const;
        void set_float(const char* name, float value) const;
        void set_vec2(const char* name, const glm::vec2& value) const;
        void set_vec3(const char* name, const glm::vec3& value) const;
        void set_vec4(const char* name, const glm::vec4& value) const;
        void set_mat4(const char* name, const glm::mat4& value) const;

        [[nodiscard]] unsigned int id() const { return program_; }
        [[nodiscard]] bool valid() const { return program_ != 0; }
        [[nodiscard]] const std::string& last_error() const { return last_error_; }

    private:
        bool link(std::span<const ShaderSource> sources);

        unsigned int program_ = 0;
        std::vector<ShaderFile> files_;
        std::string last_error_;
    };
}
