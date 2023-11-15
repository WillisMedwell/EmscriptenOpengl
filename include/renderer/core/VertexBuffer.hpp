#pragma once

#include "Libraries.hpp"

#include <cstdint>
#include <iostream>
#include <optional>
#include <span>

#include "BuildSettings.hpp"
#include "Concept.hpp"

class VertexBuffer {
private:
    std::optional<uint32_t> m_vbo;

public:
    auto init() noexcept -> void;
    auto stop() noexcept -> void;
    auto bind() noexcept -> void;
    auto unbind() noexcept -> void;

    auto loadVertices(const Concept::IsContiguousRangeWithUnderlyingType<float> auto& vertices) noexcept -> void
    {
        if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
            if (!m_vbo) {
                std::cerr << "VertexBuffer failed, trying to \"load vertices\" into an unitialised vertex buffer object.";
                exit(EXIT_FAILURE);
            }
        }
        size_t size_in_bytes = vertices.size() * sizeof(float);
        this->bind();
        glBufferData(GL_ARRAY_BUFFER, size_in_bytes, vertices.data(), GL_DYNAMIC_DRAW);
    }
    
    ~VertexBuffer();
};