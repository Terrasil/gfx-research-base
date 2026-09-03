#pragma once

namespace gfx::research {
    class FullscreenTriangle {
    public:
        FullscreenTriangle();
        FullscreenTriangle(const FullscreenTriangle&) = delete;
        FullscreenTriangle& operator=(const FullscreenTriangle&) = delete;
        FullscreenTriangle(FullscreenTriangle&& other) noexcept;
        FullscreenTriangle& operator=(FullscreenTriangle&& other) noexcept;
        ~FullscreenTriangle();

        void draw() const;

    private:
        mutable unsigned int vao_ = 0;
    };
}
