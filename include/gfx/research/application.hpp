#pragma once

#include <cstdint>
#include <string>

struct GLFWwindow;

namespace gfx::research {
    struct AppConfig {
        std::string title = "gfx research";
        int width = 1280;
        int height = 720;
        int gl_major = 4;
        int gl_minor = 6;
        bool vsync = false;
        bool debug_context = true;
        bool docking = true;
    };

    struct FrameInfo {
        std::uint64_t frame_index = 0;
        double time_seconds = 0.0;
        float delta_seconds = 0.0f;
        int width = 1;
        int height = 1;
    };

    class Application {
    public:
        explicit Application(AppConfig config = {});
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        virtual ~Application();

        int run();
        void close() const;

        [[nodiscard]] GLFWwindow* window() const { return window_; }
        [[nodiscard]] const AppConfig& config() const { return config_; }
        [[nodiscard]] float cpu_frame_milliseconds() const { return cpu_frame_milliseconds_; }
        [[nodiscard]] float fps() const { return cpu_frame_milliseconds_ > 0.0f ? 1000.0f / cpu_frame_milliseconds_ : 0.0f; }

    protected:
        virtual bool on_init() { return true; }
        virtual void on_frame(const FrameInfo& frame) = 0;
        virtual void on_gui(const FrameInfo&) {}
        virtual void on_resize(int, int) {}
        virtual void on_drop(int, const char**) {}
        virtual void on_shutdown() {}

    private:
        bool initialize();
        void shutdown();
        void handle_resize(int width, int height);

        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
        static void drop_callback(GLFWwindow* window, int count, const char** paths);

        AppConfig config_;
        GLFWwindow* window_ = nullptr;
        float cpu_frame_milliseconds_ = 0.0f;
        int framebuffer_width_ = 1;
        int framebuffer_height_ = 1;
    };
}
