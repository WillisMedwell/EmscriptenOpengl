#pragma once

#include <cstdint>

namespace BuildSettings
{
    enum class Mode : uint_fast16_t {
        release,
        debug,
        debug_without_opengl_callbacks
    };
    static constexpr auto mode = Mode::debug;
}