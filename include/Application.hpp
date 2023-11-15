#pragma once
#include <cstdlib>

void* operator new(size_t n);

#include "Libraries.hpp"

#include <expected>
#include <stdexcept>
#include <string_view>
#include <cstdint>

#include "renderer/core/OpenglContext.hpp"
#include "renderer/core/Input.hpp"
#include "renderer/Renderer.hpp"

class Application {
private: // members
    OpenglContext m_main_context;
    Scene m_scene;
    Renderer m_renderer;
    Input m_input;

public: // methods
    [[nodiscard]] auto init() noexcept -> Expected<void, std::string_view>;
    [[nodiscard]] auto run() noexcept -> Expected<void, std::string_view>;
    void stop() noexcept;
    Application() = default;
}; 