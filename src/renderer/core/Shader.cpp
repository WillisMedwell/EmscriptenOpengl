#include "renderer/3d/MeshRenderer.hpp"

#include "BuildSettings.hpp"

#include <fstream>
#include <iostream>

static auto readRawFile(const std::filesystem::path& path) -> std::string
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!std::filesystem::exists(path)) {
			std::cerr
				<< "File \""
				<< path.string()
				<< "\" does not exist.";
			exit(EXIT_FAILURE);
		}
		std::ifstream f(path);
		if (!f.is_open()) {
			constexpr static auto msg = "File is inaccessible.";
			std::cerr
				<< "File \""
				<< path.string()
				<< "\" is inaccessible.";
			exit(EXIT_FAILURE);
		}
		return std::string{ std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
	}
	else {
		std::ifstream f(path);
		return std::string{ std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
	}
}

static auto compileShader(uint32_t type, const std::string& source, const std::filesystem::path& path) -> uint32_t
{
	uint32_t shader = glCreateShader(type);
	const char* src = source.data();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		int32_t compile_status = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

		if (compile_status == GL_FALSE) {
			int32_t length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			std::vector<char> error_chars(length);
			glGetShaderInfoLog(shader, length, &length, error_chars.data());

			std::cerr
				<< "Shader failed to compile: "
				<< "\""
				<< path.string()
				<< "\".\n"
				<< std::string_view(error_chars);

			glDeleteShader(shader);
			exit(EXIT_FAILURE);
		}
	}
	return shader;
}

void Shader::uploadToGpu() noexcept
{
	std::string vert_shader_source = readRawFile(m_vert_shader_path);
	std::string frag_shader_source = readRawFile(m_frag_shader_path);

	m_program_id = glCreateProgram();

	uint32_t vert_id = compileShader(GL_VERTEX_SHADER, vert_shader_source, m_vert_shader_path);
	uint32_t frag_id = compileShader(GL_FRAGMENT_SHADER, frag_shader_source, m_frag_shader_path);

#if BUILD_TARGET == NATIVE_BUILD
	std::optional<uint32_t> geo_id = m_geo_shader_path.transform([&](std::filesystem::path geo_path) {
		std::string geo_shader_source = readRawFile(geo_path);
		return compileShader(GL_GEOMETRY_SHADER, geo_shader_source, geo_path);
		});
#endif

	glAttachShader(m_program_id.value(), vert_id);

#if BUILD_TARGET == NATIVE_BUILD
	geo_id.and_then([&](uint32_t id) {
		glAttachShader(m_program_id.value(), id);
		return std::optional(id);
		});
#endif
	glAttachShader(m_program_id.value(), frag_id);
	glLinkProgram(m_program_id.value());
	glValidateProgram(m_program_id.value());

	glDeleteShader(vert_id);
	glDeleteShader(frag_id);

#if BUILD_TARGET == NATIVE_BUILD
	geo_id.and_then([&](uint32_t id) {
		glDeleteShader(id);
		return std::optional(id);
		});
#endif 

	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		int32_t validation_status = GL_FALSE;
		glGetProgramiv(m_program_id.value(), GL_VALIDATE_STATUS, &validation_status);
		if (!validation_status) {
			GLint infoLogLength = 0;
			glGetProgramiv(m_program_id.value(), GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 0) {
				std::vector<char> infoLog(infoLogLength);
				glGetProgramInfoLog(m_program_id.value(), infoLogLength, nullptr, &infoLog[0]);
				std::cerr << "Unable to validate the program when constructing the shader. Error: " << &infoLog[0];
			}
			else {
				std::cerr << "Unable to validate the program when constructing the shader. No additional info.";
			}
			exit(EXIT_FAILURE);
		}
	}
	glUseProgram(m_program_id.value());
}
void Shader::bind() noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_program_id) {
			std::cerr << "Trying to bind shader that isn't on GPU.";
			exit(EXIT_FAILURE);
		}
	}
	glUseProgram(m_program_id.value());
}
void Shader::unbind() noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_program_id) {
			std::cerr << "Trying to unbind shader that isn't on GPU.";
			assert(false);
			exit(EXIT_FAILURE);
		}
	}
	glUseProgram(0);
}
void Shader::offloadFromGpu() noexcept
{
	if (m_program_id) {
		glDeleteProgram(m_program_id.value());
	}
	m_program_id = std::nullopt;
	m_uniforms_lookup.clear();
}
void Shader::init(std::filesystem::path vert_shader_path, std::filesystem::path frag_shader_path, std::optional<std::filesystem::path> geo_shader_path) noexcept
{
	m_vert_shader_path = std::move(vert_shader_path);
	m_frag_shader_path = std::move(frag_shader_path);
	m_geo_shader_path = std::move(geo_shader_path);
}
void Shader::reload() noexcept
{
	bool needs_uploading = m_program_id.has_value();
	offloadFromGpu();

	if (needs_uploading) {
		uploadToGpu();
	}
}
void Shader::stop() noexcept
{
	offloadFromGpu();
	m_vert_shader_path.clear();
	m_frag_shader_path.clear();
}
auto Shader::setUniform(const std::string_view& key, const glm::mat4& value) -> Expected<void, std::string_view>
{
	auto setUniformAtIndex = [&](int32_t index) -> void {
		this->bind();
		glUniformMatrix4fv(index, 1, GL_FALSE, (const float*)&value[0][0]);
		};

	if (auto result = getUniformIndex(key).OnValue(setUniformAtIndex); result.HasError()) {
		return { result.Error() };
	}
	else {
		return {};
	}
}
auto Shader::setUniform(const std::string_view& key, const glm::vec3& value) -> Expected<void, std::string_view>
{
	auto setUniformAtIndex = [&](int32_t index) -> void {
		this->bind();
		glUniform3f(index, value.x, value.y, value.z);
		};
	if (auto result = getUniformIndex(key).OnValue(setUniformAtIndex); result.HasError()) {
		return { result.Error() };
	}
	else {
		return {};
	}
}
auto Shader::setUniform(const std::string_view& key, const float value) -> Expected<void, std::string_view>
{
	auto setUniformAtIndex = [&](int32_t index) -> void {
		this->bind();
		glUniform1f(index, value);
		};
	if (auto result = getUniformIndex(key).OnValue(setUniformAtIndex); result.HasError()) {
		return { result.Error() };
	}
	else {
		return {};
	}
}
auto Shader::setUniform(const std::string_view& key, const int32_t value) -> Expected<void, std::string_view>
{
	auto setUniformAtIndex = [&](int32_t index) -> void {
		this->bind();
		glUniform1i(index, value);
		};
	if (auto result = getUniformIndex(key).OnValue(setUniformAtIndex); result.HasError()) {
		return { result.Error() };
	}
	else {
		return {};
	}
}


Shader::~Shader()
{
	this->stop();
}

auto Shader::getUniformIndex(const std::string_view& key) -> Expected<int32_t, std::string_view>
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_program_id) {
			std::cerr << "Trying to find uniform of shader that doesn't have an program id.";
			exit(EXIT_FAILURE);
		}
	}

	auto hash = std::hash<std::string_view>{}(key);

	if (m_uniforms_lookup.contains(hash)) {
		return m_uniforms_lookup[hash];
	}

	int32_t uniform_index = glGetUniformLocation(m_program_id.value(), key.data());

	if (uniform_index == -1) {
		return { key };
	}

	m_uniforms_lookup.emplace(hash, uniform_index);
	return uniform_index;
}
