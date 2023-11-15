#include "renderer/core/VertexBuffer.hpp"

auto VertexBuffer::init() noexcept -> void
{
    this->stop();
    uint32_t temp_vbo;
    glGenBuffers(1, &temp_vbo);
    if (temp_vbo != 0) {
        m_vbo = temp_vbo;
    } else {
        m_vbo = std::nullopt;
    }
}

auto VertexBuffer::stop() noexcept -> void
{
    if (m_vbo) {
        this->unbind();
        glDeleteBuffers(1, &m_vbo.value());
        m_vbo = std::nullopt;
    }
}

auto VertexBuffer::bind() noexcept -> void
{
    if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
        if (!m_vbo) {
            std::cerr << "VertexBuffer failed, trying to \"bind\" an unitialised vertex buffer object.";
            exit(EXIT_FAILURE);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo.value());
}

auto VertexBuffer::unbind() noexcept -> void
{
    if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
        if (!m_vbo) {
            std::cerr << "VertexBuffer failed, trying to \"unbind\" an unitialised vertex buffer object.";
            exit(EXIT_FAILURE);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
    this->stop();
}