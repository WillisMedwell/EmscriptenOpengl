#pragma once

#include "Libraries.hpp"

#include <cstdint>
#include <iostream>
#include <optional>
#include <span>

#include "BuildSettings.hpp"
#include "Concept.hpp"
#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"

class VertexArray {
private:
    std::optional<uint32_t> m_vao;

public:
    auto init() noexcept -> void;
    auto stop() noexcept -> void;
    auto bind() noexcept -> void;
    auto unbind() noexcept -> void;

    auto attachBufferAndLayout(VertexBuffer& vb, VertexBufferLayout& layout) -> void;

    ~VertexArray();
};