#pragma once

#include "Libraries.hpp"

#include <filesystem>
#include <unordered_map>
#include <cstdint>
#include <optional>

#include "Expected.hpp"

class Shader
{
	std::optional<int32_t> m_program_id;
    std::unordered_map<size_t, int32_t> m_uniforms_lookup;
	
	std::filesystem::path m_vert_shader_path;
	std::filesystem::path m_frag_shader_path;
	std::optional<std::filesystem::path> m_geo_shader_path;
public:
	void uploadToGpu() noexcept;
	void bind() noexcept;
	void unbind() noexcept;

	void offloadFromGpu() noexcept;

	void init(std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, std::optional<std::filesystem::path> geo_shader_path = std::nullopt) noexcept;
	void reload() noexcept;
	void stop() noexcept;

	auto setUniform(const std::string_view& key, const glm::mat4& value) -> Expected<void, std::string_view>;
	auto setUniform(const std::string_view& key, const glm::vec3& value) -> Expected<void, std::string_view>;
	auto setUniform(const std::string_view& key, const float value) -> Expected<void, std::string_view>;
	auto setUniform(const std::string_view& key, const int32_t value) -> Expected<void, std::string_view>;

	~Shader();
private:
	auto getUniformIndex(const std::string_view& key) -> Expected<int32_t, std::string_view>;
};