#include "renderer/core/IndexBuffer.hpp"

#include "BuildSettings.hpp"

auto IndexBuffer::init() noexcept -> void
{
	stop();

	uint32_t temp_ibo;
	glGenBuffers(1, &temp_ibo);

	if (temp_ibo != 0) {
		m_ibo = temp_ibo;
	}
	else {
		m_ibo = std::nullopt;
	}
	m_index_count = 0;
}

auto IndexBuffer::stop() noexcept -> void
{
	if (m_ibo) {
		this->unbind();
		glDeleteBuffers(1, &m_ibo.value());
		m_ibo = std::nullopt;
	}
}

auto IndexBuffer::bind() noexcept -> void
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_ibo) {
			std::cerr << "IndexBuffer failed, trying to \"bind\" an unitialised index buffer object.";
			exit(EXIT_FAILURE);
		}
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo.value());
}

auto IndexBuffer::unbind() noexcept -> void
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_ibo) {
			std::cerr << "IndexBuffer failed, trying to \"unbind\" an unitialised index buffer object.";
			exit(EXIT_FAILURE);
		}
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
	this->stop();
}