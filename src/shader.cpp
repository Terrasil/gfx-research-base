#include <gfx/research/shader.hpp>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <utility>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

namespace gfx::research {
    namespace {
        GLenum to_gl_stage(ShaderStage stage) {
            switch (stage) {
                case ShaderStage::Vertex: return GL_VERTEX_SHADER;
                case ShaderStage::TessellationControl: return GL_TESS_CONTROL_SHADER;
                case ShaderStage::TessellationEvaluation: return GL_TESS_EVALUATION_SHADER;
                case ShaderStage::Geometry: return GL_GEOMETRY_SHADER;
                case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
                case ShaderStage::Compute: return GL_COMPUTE_SHADER;
            }
            return GL_VERTEX_SHADER;
        }

        std::string read_text(const std::filesystem::path& path) {
            std::ifstream file(path, std::ios::binary);
            if (!file) return {};
            std::ostringstream stream;
            stream << file.rdbuf();
            return stream.str();
        }
    }

    Shader::Shader(std::span<const ShaderFile> files) { load(files); }

    Shader::Shader(const char* vertex_source, const char* fragment_source) {
        const ShaderSource sources[] = {
            {ShaderStage::Vertex, vertex_source},
            {ShaderStage::Fragment, fragment_source}
        };
        load(sources);
    }

    Shader::Shader(Shader&& other) noexcept { *this = std::move(other); }

    Shader& Shader::operator=(Shader&& other) noexcept {
        if (this == &other) return *this;
        clear();
        program_ = std::exchange(other.program_, 0);
        files_ = std::move(other.files_);
        last_error_ = std::move(other.last_error_);
        return *this;
    }

    Shader::~Shader() { clear(); }

    bool Shader::load(std::span<const ShaderFile> files) {
        std::vector<ShaderSource> sources;
        sources.reserve(files.size());
        for (const auto& file : files) {
            std::string source = read_text(file.path);
            if (source.empty()) {
                last_error_ = "Unable to read shader: " + file.path.string();
                std::fprintf(stderr, "%s\n", last_error_.c_str());
                return false;
            }
            sources.push_back({file.stage, std::move(source)});
        }
        if (!link(sources)) return false;
        files_.assign(files.begin(), files.end());
        return true;
    }

    bool Shader::load(std::span<const ShaderSource> sources) {
        files_.clear();
        return link(sources);
    }

    bool Shader::reload() {
        if (files_.empty()) return false;
        return load(files_);
    }

    bool Shader::link(std::span<const ShaderSource> sources) {
        std::vector<GLuint> shaders;
        shaders.reserve(sources.size());
        last_error_.clear();

        for (const auto& stage : sources) {
            const GLuint shader = glCreateShader(to_gl_stage(stage.stage));
            const char* ptr = stage.source.c_str();
            glShaderSource(shader, 1, &ptr, nullptr);
            glCompileShader(shader);

            GLint ok = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                GLint length = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                std::string log(static_cast<std::size_t>(std::max(length, 1)), '\0');
                glGetShaderInfoLog(shader, length, nullptr, log.data());
                last_error_ = log;
                std::fprintf(stderr, "Shader compile error:\n%s\n", last_error_.c_str());
                glDeleteShader(shader);
                for (const GLuint compiled : shaders) glDeleteShader(compiled);
                return false;
            }
            shaders.push_back(shader);
        }

        const GLuint program = glCreateProgram();
        for (const GLuint shader : shaders) glAttachShader(program, shader);
        glLinkProgram(program);
        for (const GLuint shader : shaders) glDeleteShader(shader);

        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::string log(static_cast<std::size_t>(std::max(length, 1)), '\0');
            glGetProgramInfoLog(program, length, nullptr, log.data());
            last_error_ = log;
            std::fprintf(stderr, "Program link error:\n%s\n", last_error_.c_str());
            glDeleteProgram(program);
            return false;
        }

        if (program_) glDeleteProgram(program_);
        program_ = program;
        return true;
    }

    void Shader::clear() {
        if (program_) glDeleteProgram(program_);
        program_ = 0;
    }

    void Shader::bind() const { glUseProgram(program_); }

    void Shader::set_bool(const char* name, bool value) const { set_int(name, value ? 1 : 0); }
    void Shader::set_int(const char* name, int value) const { glUniform1i(glGetUniformLocation(program_, name), value); }
    void Shader::set_uint(const char* name, unsigned int value) const { glUniform1ui(glGetUniformLocation(program_, name), value); }
    void Shader::set_float(const char* name, float value) const { glUniform1f(glGetUniformLocation(program_, name), value); }
    void Shader::set_vec2(const char* name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(value)); }
    void Shader::set_vec3(const char* name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(value)); }
    void Shader::set_vec4(const char* name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(value)); }
    void Shader::set_mat4(const char* name, const glm::mat4& value) const { glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(value)); }
}
