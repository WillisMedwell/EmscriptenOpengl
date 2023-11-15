#pragma once

#include "Libraries.hpp"

#include <cstdint>
#include <iostream>
#include <optional>
#include <span>

#include "BuildSettings.hpp"
#include "Concept.hpp"

class IndexBuffer {
private:
	std::optional<uint32_t> m_ibo;
	uint32_t m_index_count;
public:
	~IndexBuffer();

	auto init() noexcept -> void;
	auto stop() noexcept -> void;
	auto bind() noexcept -> void;
	auto unbind() noexcept -> void;

	auto loadIndices(const Concept::IsContiguousRangeWithUnderlyingType<uint32_t> auto& indices) noexcept -> void
	{
		if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
			if (!m_ibo) {
				std::cerr << "IndexBuffer failed, trying to \"load indices\" into an unitialised index buffer object.";
				exit(EXIT_FAILURE);
			}
		}
		size_t size_in_bytes = indices.size() * sizeof(uint32_t);
		this->bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, indices.data(), GL_DYNAMIC_DRAW);
		m_index_count = indices.size();
	}
	auto getIndexCount() const noexcept -> uint32_t { return m_index_count; }
};