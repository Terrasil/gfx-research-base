#include <gfx/research/orbit_camera.hpp>

#include <algorithm>
#include <cmath>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace gfx::research {
    glm::vec3 OrbitCamera::position() const {
        const float pitch = std::clamp(pitch_, -1.55334f, 1.55334f);
        const float distance = std::max(distance_, 0.001f);
        return target_ + glm::vec3(
            std::cos(pitch) * std::sin(yaw_) * distance,
            std::sin(pitch) * distance,
            std::cos(pitch) * std::cos(yaw_) * distance
        );
    }

    glm::mat4 OrbitCamera::view() const {
        return glm::lookAt(position(), target_, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 OrbitCamera::projection(float aspect) const {
        return glm::perspective(glm::radians(fov_degrees_), std::max(aspect, 0.001f), near_plane_, far_plane_);
    }

    glm::mat4 OrbitCamera::view_projection(float aspect) const {
        return projection(aspect) * view();
    }
}
