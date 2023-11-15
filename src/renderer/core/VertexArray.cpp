#include "renderer/core/VertexArray.hpp"

auto VertexArray::init() noexcept -> void
{
	this->stop();

	uint32_t temp_vao;
	glGenVertexArrays(1, &temp_vao);

	if (temp_vao != 0) {
		m_vao = temp_vao;
	}
	else {
		m_vao = std::nullopt;
	}
}

auto VertexArray::stop() noexcept -> void
{
	if (m_vao) {
		this->unbind();
		glDeleteVertexArrays(1, &m_vao.value());
		m_vao = std::nullopt;
	}
}

auto VertexArray::bind() noexcept -> void
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_vao) {
			std::cerr << "VertexBuffer failed, trying to \"bind\" an unitialised vertex array.\n" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	glBindVertexArray(m_vao.value());
}

auto VertexArray::unbind() noexcept -> void
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_vao) {
			std::cerr << "VertexBuffer failed, trying to \"unbind\" an unitialised vertex array.\n" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	glBindVertexArray(0);
}

auto VertexArray::attachBufferAndLayout(VertexBuffer& vb, VertexBufferLayout& layout) -> void
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_vao) {
			std::cerr << "VertexBuffer failed, trying to \"attach\" to an unitialised vertex array.\n" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	this->bind();
	vb.bind();

#if BUILD_TARGET == NATIVE_BUILD
	uint32_t offset = 0;
	for (size_t i = 0; i < layout.elements.size(); i++) {
		const auto& element = layout.elements[i];
		glEnableVertexArrayAttrib(m_vao.value(), i);
		glVertexAttribPointer(i, element.count, element.type, element.normalised, layout.stride, (const void*)offset);
		offset += element.count * element.getTypeSize();
	}
#elif BUILD_TARGET == WEB_BUILD
	uint32_t offset = 0;
	for (size_t i = 0; i < layout.elements.size(); i++) {
		const auto& element = layout.elements[i];
		glEnableVertexAttribArray(i); 
		glVertexAttribPointer(i, element.count, element.type, element.normalised, layout.stride, reinterpret_cast<const void*>(offset));
		offset += element.count * element.getTypeSize();
	}
#endif
}

VertexArray::~VertexArray()
{
	this->stop();
}