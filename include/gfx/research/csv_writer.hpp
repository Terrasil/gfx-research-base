#pragma once

#include <filesystem>
#include <fstream>

namespace gfx::research {
    class CsvWriter {
    public:
        CsvWriter() = default;
        explicit CsvWriter(const std::filesystem::path& path) { open(path); }

        bool open(const std::filesystem::path& path) {
            if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
            stream_.open(path, std::ios::out | std::ios::trunc);
            return stream_.is_open();
        }

        template<typename... T>
        void row(const T&... values) {
            bool first = true;
            ((stream_ << (first ? "" : ",") << values, first = false), ...);
            stream_ << '\n';
        }

        void flush() { stream_.flush(); }
        [[nodiscard]] bool is_open() const { return stream_.is_open(); }

    private:
        std::ofstream stream_;
    };
}
