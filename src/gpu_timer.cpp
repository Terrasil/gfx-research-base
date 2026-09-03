#include <gfx/research/gpu_timer.hpp>

#include <glad/gl.h>

namespace gfx::research {
    GpuTimer::GpuTimer() = default;
    GpuTimer::~GpuTimer() {
        if (queries_[0]) glDeleteQueries(static_cast<GLsizei>(queries_.size()), queries_.data());
    }

    void GpuTimer::ensure_created() {
        if (!queries_[0]) glGenQueries(static_cast<GLsizei>(queries_.size()), queries_.data());
    }

    void GpuTimer::begin() {
        if (active_) return;
        ensure_created();
        active_ = true;
        glQueryCounter(queries_[slot_ * 2], GL_TIMESTAMP);
    }

    void GpuTimer::end() {
        if (!active_) return;
        glQueryCounter(queries_[slot_ * 2 + 1], GL_TIMESTAMP);
        active_ = false;

        const std::size_t resolve_slot = (slot_ + 1) % slot_count;
        GLint available = GL_FALSE;
        glGetQueryObjectiv(queries_[resolve_slot * 2 + 1], GL_QUERY_RESULT_AVAILABLE, &available);
        if (available) {
            GLuint64 begin = 0;
            GLuint64 end = 0;
            glGetQueryObjectui64v(queries_[resolve_slot * 2], GL_QUERY_RESULT, &begin);
            glGetQueryObjectui64v(queries_[resolve_slot * 2 + 1], GL_QUERY_RESULT, &end);
            if (end >= begin) {
                last_milliseconds_ = static_cast<double>(end - begin) / 1'000'000.0;
                has_result_ = true;
            }
        }
        slot_ = (slot_ + 1) % slot_count;
    }
}
