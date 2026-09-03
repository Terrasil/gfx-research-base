#pragma once

#include <array>
#include <cstddef>

namespace gfx::research {
    class GpuTimer {
    public:
        GpuTimer();
        GpuTimer(const GpuTimer&) = delete;
        GpuTimer& operator=(const GpuTimer&) = delete;
        ~GpuTimer();

        void begin();
        void end();

        [[nodiscard]] double milliseconds() const { return last_milliseconds_; }
        [[nodiscard]] bool has_result() const { return has_result_; }

    private:
        void ensure_created();

        static constexpr std::size_t slot_count = 4;
        std::array<unsigned int, slot_count * 2> queries_{};
        std::size_t slot_ = 0;
        double last_milliseconds_ = 0.0;
        bool has_result_ = false;
        bool active_ = false;
    };
}
