#include <gfx/research/application.hpp>
#include <gfx/research/gpu_timer.hpp>
#include <gfx/research/model.hpp>
#include <gfx/research/orbit_camera.hpp>
#include <gfx/research/primitives.hpp>
#include <gfx/research/screenshot.hpp>
#include <gfx/research/shader.hpp>

#include <array>
#include <cstdio>
#include <filesystem>

#include <glad/gl.h>
#include <imgui.h>
#include <glm/ext/matrix_transform.hpp>

namespace {
    class Example final : public gfx::research::Application {
    public:
        Example() : Application({.title = "gfx-research-base example"}) {}

    private:
        bool on_init() override {
            const std::filesystem::path root = GFX_RESEARCH_BASE_SOURCE_DIR;
            const gfx::research::ShaderFile shader_files[] = {
                {gfx::research::ShaderStage::Vertex, root / "example/shaders/model.vert"},
                {gfx::research::ShaderStage::Fragment, root / "example/shaders/model.frag"}
            };
            if (!shader_.load(shader_files)) return false;
            fallback_ = gfx::research::make_uv_sphere(1.0f, 64, 32);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            return true;
        }

        void on_drop(int count, const char** paths) override {
            if (count > 0) load_model(paths[0]);
        }

        void on_frame(const gfx::research::FrameInfo& frame) override {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, frame.width, frame.height);
            glClearColor(0.055f, 0.065f, 0.085f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glPolygonMode(GL_FRONT_AND_BACK, wireframe_ ? GL_LINE : GL_FILL);

            const float aspect = static_cast<float>(frame.width) / static_cast<float>(frame.height);
            const glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_));
            shader_.bind();
            shader_.set_mat4("uModel", model_matrix);
            shader_.set_mat4("uMvp", camera_.view_projection(aspect) * model_matrix);
            shader_.set_vec3("uEye", camera_.position());

            gpu_timer_.begin();
            if (model_.vertex_count()) model_.draw();
            else fallback_.draw();
            gpu_timer_.end();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        void on_gui(const gfx::research::FrameInfo& frame) override {
            ImGui::Begin("Research base");
            ImGui::Text("CPU %.3f ms | %.1f FPS", cpu_frame_milliseconds(), fps());
            ImGui::Text("GPU %.3f ms", gpu_timer_.milliseconds());
            ImGui::Separator();
            ImGui::InputText("Model", path_.data(), path_.size());
            if (ImGui::Button("load")) load_model(path_.data());
            ImGui::SameLine();
            if (ImGui::Button("clear")) model_.clear();
            ImGui::SameLine();
            if (ImGui::Button("reload shaders")) shader_.reload();
            ImGui::Text("Vertices %zu | indices %zu", model_.vertex_count(), model_.index_count());
            ImGui::SliderFloat("distance", &camera_.distance(), 0.1f, 100.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderAngle("yaw", &camera_.yaw(), -180.0f, 180.0f);
            ImGui::SliderAngle("pitch", &camera_.pitch(), -89.0f, 89.0f);
            ImGui::SliderFloat("Scale", &scale_, 0.001f, 100.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::Checkbox("Wireframe", &wireframe_);
            if (ImGui::Button("Screenshot")) gfx::research::save_framebuffer_png("results/base-example.png", frame.width, frame.height);
            ImGui::End();
        }

        void load_model(const std::filesystem::path& path) {
            if (model_.load(path)) std::snprintf(path_.data(), path_.size(), "%s", path.string().c_str());
        }

        gfx::research::Shader shader_;
        gfx::research::Model model_;
        gfx::research::Mesh fallback_;
        gfx::research::OrbitCamera camera_;
        gfx::research::GpuTimer gpu_timer_;
        std::array<char, 1024> path_{};
        float scale_ = 1.0f;
        bool wireframe_ = false;
    };
}

int main() {
    return Example{}.run();
}
