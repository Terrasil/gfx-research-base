#include <gfx/research/application.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <utility>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace gfx::research {
    namespace {
        void APIENTRY debug_callback(GLenum, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*) {
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
            std::fprintf(stderr, "OpenGL[%u] type=0x%x severity=0x%x: %s\n", id, type, severity, message);
        }
    }

    Application::Application(AppConfig config) : config_(std::move(config)) {}
    Application::~Application() { shutdown(); }

    bool Application::initialize() {
        if (!glfwInit()) {
            std::fprintf(stderr, "GLFW initialization failed\n");
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config_.gl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config_.gl_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, config_.debug_context ? GLFW_TRUE : GLFW_FALSE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

        window_ = glfwCreateWindow(config_.width, config_.height, config_.title.c_str(), nullptr, nullptr);
        if (!window_) {
            std::fprintf(stderr, "GLFW window creation failed\n");
            glfwTerminate();
            return false;
        }

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, &Application::framebuffer_size_callback);
        glfwSetDropCallback(window_, &Application::drop_callback);
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(config_.vsync ? 1 : 0);

        if (!gladLoadGL(glfwGetProcAddress)) {
            std::fprintf(stderr, "GLAD initialization failed\n");
            shutdown();
            return false;
        }

        if (config_.debug_context && GLAD_GL_VERSION_4_3) {
            GLint context_flags = 0;
            glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
            if ((context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(debug_callback, nullptr);
            }
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        if (config_.docking) {
#ifdef IMGUI_HAS_DOCK
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#else
            std::fprintf(stderr, "Dear ImGui docking requested, but the selected ImGui build has no docking support\n");
#endif
        }
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        glfwGetFramebufferSize(window_, &framebuffer_width_, &framebuffer_height_);
        framebuffer_width_ = std::max(framebuffer_width_, 1);
        framebuffer_height_ = std::max(framebuffer_height_, 1);
        glViewport(0, 0, framebuffer_width_, framebuffer_height_);
        return true;
    }

    int Application::run() {
        if (!initialize()) return 1;
        if (!on_init()) return 2;

        using Clock = std::chrono::steady_clock;
        auto previous = Clock::now();
        const auto start = previous;
        std::uint64_t frame_index = 0;

        while (!glfwWindowShouldClose(window_)) {
            const auto frame_start = Clock::now();
            const float delta = std::chrono::duration<float>(frame_start - previous).count();
            previous = frame_start;

            glfwPollEvents();
            glfwGetFramebufferSize(window_, &framebuffer_width_, &framebuffer_height_);
            framebuffer_width_ = std::max(framebuffer_width_, 1);
            framebuffer_height_ = std::max(framebuffer_height_, 1);

            FrameInfo frame;
            frame.frame_index = frame_index++;
            frame.time_seconds = std::chrono::duration<double>(frame_start - start).count();
            frame.delta_seconds = delta;
            frame.width = framebuffer_width_;
            frame.height = framebuffer_height_;

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            on_frame(frame);
            on_gui(frame);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window_);

            const auto frame_end = Clock::now();
            const float ms = std::chrono::duration<float, std::milli>(frame_end - frame_start).count();
            cpu_frame_milliseconds_ = cpu_frame_milliseconds_ == 0.0f ? ms : cpu_frame_milliseconds_ * 0.9f + ms * 0.1f;
        }

        on_shutdown();
        return 0;
    }

    void Application::close() const {
        if (window_) glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }

    void Application::shutdown() {
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        if (window_) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        glfwTerminate();
    }

    void Application::handle_resize(int width, int height) {
        framebuffer_width_ = std::max(width, 1);
        framebuffer_height_ = std::max(height, 1);
        glViewport(0, 0, framebuffer_width_, framebuffer_height_);
        on_resize(framebuffer_width_, framebuffer_height_);
    }

    void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        if (auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window))) app->handle_resize(width, height);
    }

    void Application::drop_callback(GLFWwindow* window, int count, const char** paths) {
        if (auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window))) app->on_drop(count, paths);
    }
}
