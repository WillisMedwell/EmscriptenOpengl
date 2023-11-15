#pragma once

#include "Libraries.hpp"

#include <glm/glm.hpp>

#include <string_view>
#include <cstdint>
#include <stdexcept>
#include <tuple>

#include "Expected.hpp"

class OpenglContext {
    uint32_t m_width;
    uint32_t m_height;
    GLFWwindow* m_window;
public:
    OpenglContext();

    auto init(std::string_view name, uint32_t width, uint32_t height) noexcept -> Expected<void, std::string_view>;
    void clearBuffer() noexcept;
    void swapBuffers() noexcept;
    void setBackgroundColour(glm::u8vec3 rgb) noexcept;
    void setBackgroundColour(glm::u8vec4 rgba) noexcept;
    bool shouldClose() noexcept;
    bool isInit() noexcept;
    void pollEvents() noexcept;
    void stop() noexcept;

    auto getScreenDimensions() const noexcept -> std::tuple<uint32_t, uint32_t> { return {m_width, m_height}; }
    
    auto getWindowHandle() -> void* { return m_window; }

    ~OpenglContext();
};