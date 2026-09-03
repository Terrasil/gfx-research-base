#pragma once

#include <filesystem>

namespace gfx::research {
    class Texture2D {
    public:
        Texture2D() = default;
        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D(Texture2D&& other) noexcept;
        Texture2D& operator=(Texture2D&& other) noexcept;
        ~Texture2D();

        bool create(int width, int height, unsigned int internal_format, int levels = 1);
        bool load(const std::filesystem::path& path, bool srgb = false, bool generate_mips = true);
        void upload(int level, unsigned int format, unsigned int type, const void* data);
        void generate_mipmaps();
        void bind(unsigned int unit) const;
        void clear();

        void set_filters(unsigned int min_filter, unsigned int mag_filter);
        void set_wrap(unsigned int wrap_s, unsigned int wrap_t);

        [[nodiscard]] unsigned int id() const { return id_; }
        [[nodiscard]] int width() const { return width_; }
        [[nodiscard]] int height() const { return height_; }
        [[nodiscard]] int levels() const { return levels_; }

    private:
        unsigned int id_ = 0;
        int width_ = 0;
        int height_ = 0;
        int levels_ = 1;
        unsigned int internal_format_ = 0;
    };
}
