#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace gfx::research {
    class OrbitCamera {
    public:
        void set_target(const glm::vec3& target) { target_ = target; }
        void set_distance(float distance) { distance_ = distance; }
        void set_yaw(float yaw) { yaw_ = yaw; }
        void set_pitch(float pitch) { pitch_ = pitch; }
        void set_fov(float degrees) { fov_degrees_ = degrees; }
        void set_clip(float near_plane, float far_plane) { near_plane_ = near_plane; far_plane_ = far_plane; }

        [[nodiscard]] glm::vec3 position() const;
        [[nodiscard]] glm::mat4 view() const;
        [[nodiscard]] glm::mat4 projection(float aspect) const;
        [[nodiscard]] glm::mat4 view_projection(float aspect) const;

        [[nodiscard]] glm::vec3& target() { return target_; }
        [[nodiscard]] float& distance() { return distance_; }
        [[nodiscard]] float& yaw() { return yaw_; }
        [[nodiscard]] float& pitch() { return pitch_; }
        [[nodiscard]] float& fov() { return fov_degrees_; }

    private:
        glm::vec3 target_{0.0f};
        float distance_ = 4.0f;
        float yaw_ = 0.6f;
        float pitch_ = 0.35f;
        float fov_degrees_ = 60.0f;
        float near_plane_ = 0.01f;
        float far_plane_ = 5000.0f;
    };
}
