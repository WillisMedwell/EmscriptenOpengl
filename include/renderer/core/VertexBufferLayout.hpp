#pragma once

#include "Libraries.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

struct VertexBufferElement {
    uint32_t count;
    uint32_t type;
    uint32_t normalised;

    constexpr auto getTypeSize() const -> uint32_t
    {
        switch (type) {
        case GL_FLOAT:
            return sizeof(float);
        case GL_UNSIGNED_INT:
            return sizeof(uint32_t);
        default:
            assert(false);
            return 0;
        }
    }
};

class VertexBufferLayout {
public:
    std::vector<VertexBufferElement> elements;
    uint32_t stride;

public:
    constexpr VertexBufferLayout()
        : elements({})
        , stride(0)
    {
    }

    template <typename T>
    constexpr void push(uint32_t count)
    {
        throw std::runtime_error("\'VertexBufferLayout.hpp\': Unimplemented buffer layout push");
    }
    template <>
    constexpr void push<float>(uint32_t count)
    {
        elements.push_back({ count, GL_FLOAT, GL_FALSE });
        stride += sizeof(float) * count;
    }
    template <>
    constexpr void push<uint32_t>(uint32_t count)
    {
        elements.push_back({ count, GL_UNSIGNED_INT, GL_FALSE });
        stride += sizeof(uint32_t) * count;
    }
};
