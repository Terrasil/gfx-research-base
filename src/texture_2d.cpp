#include <gfx/research/texture_2d.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <utility>

#include <glad/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gfx::research {
    Texture2D::Texture2D(Texture2D&& other) noexcept { *this = std::move(other); }

    Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
        if (this == &other) return *this;
        clear();
        id_ = std::exchange(other.id_, 0);
        width_ = std::exchange(other.width_, 0);
        height_ = std::exchange(other.height_, 0);
        levels_ = std::exchange(other.levels_, 1);
        internal_format_ = std::exchange(other.internal_format_, 0);
        return *this;
    }

    Texture2D::~Texture2D() { clear(); }

    bool Texture2D::create(int width, int height, unsigned int internal_format, int levels) {
        clear();
        if (width <= 0 || height <= 0 || levels <= 0) return false;
        width_ = width;
        height_ = height;
        levels_ = levels;
        internal_format_ = internal_format;
        glCreateTextures(GL_TEXTURE_2D, 1, &id_);
        glTextureStorage2D(id_, levels, internal_format, width, height);
        glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
        return true;
    }

    bool Texture2D::load(const std::filesystem::path& path, bool srgb, bool generate_mips) {
        int channels = 0;
        stbi_set_flip_vertically_on_load(1);
        unsigned char* pixels = stbi_load(path.string().c_str(), &width_, &height_, &channels, 4);
        if (!pixels) {
            std::fprintf(stderr, "Texture load failed: %s\n", path.string().c_str());
            return false;
        }

        const int max_dimension = std::max(width_, height_);
        const int levels = generate_mips ? 1 + static_cast<int>(std::floor(std::log2(static_cast<float>(max_dimension)))) : 1;
        create(width_, height_, srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8, levels);
        glTextureSubImage2D(id_, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        stbi_image_free(pixels);
        if (generate_mips) generate_mipmaps();
        return true;
    }

    void Texture2D::upload(int level, unsigned int format, unsigned int type, const void* data) {
        if (!id_ || level < 0 || level >= levels_) return;
        const int width = std::max(1, width_ >> level);
        const int height = std::max(1, height_ >> level);
        glTextureSubImage2D(id_, level, 0, 0, width, height, format, type, data);
    }

    void Texture2D::generate_mipmaps() { if (id_ && levels_ > 1) glGenerateTextureMipmap(id_); }
    void Texture2D::bind(unsigned int unit) const { glBindTextureUnit(unit, id_); }
    void Texture2D::set_filters(unsigned int min_filter, unsigned int mag_filter) {
        if (!id_) return;
        glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min_filter));
        glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag_filter));
    }
    void Texture2D::set_wrap(unsigned int wrap_s, unsigned int wrap_t) {
        if (!id_) return;
        glTextureParameteri(id_, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrap_s));
        glTextureParameteri(id_, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrap_t));
    }
    void Texture2D::clear() {
        if (id_) glDeleteTextures(1, &id_);
        id_ = 0;
        width_ = height_ = 0;
        levels_ = 1;
        internal_format_ = 0;
    }
}
