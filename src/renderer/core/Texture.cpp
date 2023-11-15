#include "renderer/core/Texture.hpp"

#include "BuildSettings.hpp"

#include <lodepng.h>
#include <iostream>
#include <filesystem>

Texture::Texture()
	: m_texture_path("")
	, m_image(std::nullopt)
	, m_texture_id(std::nullopt)
	, m_bound_slot(std::nullopt)
{
}

void Texture::uploadToGpu() noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release)
	{
		if (!m_image) {
			std::cerr << "Trying to \"bind\" a texture that has no image loaded";
			exit(EXIT_FAILURE);
		}
	}

	if (!m_texture_id) {
		m_texture_id = 0;
		glGenTextures(1, &(m_texture_id.value()));
		glBindTexture(GL_TEXTURE_2D, m_texture_id.value());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, m_image.value().width, m_image.value().height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)m_image.value().data.data());
	}
	else {
		offloadFromGpu();
		uploadToGpu();
	}
}
void Texture::offloadFromGpu() noexcept
{
	unbind();
	if (m_texture_id) {
		glDeleteTextures(1, &(m_texture_id.value()));
	}
	m_texture_id = std::nullopt;
	m_bound_slot = std::nullopt;
}
void Texture::bind(uint32_t slot) noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release)
	{
		if (!m_texture_id) {
			std::cerr << "Trying to \"bind\" a texture that has no texture id";
			exit(EXIT_FAILURE);
		}
	}
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_texture_id.value_or(0));
}
void Texture::unbind() noexcept
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::init(std::filesystem::path texture_path) noexcept
{
	m_texture_path = std::move(texture_path);

	// validate path
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release)
	{
		if (!std::filesystem::exists(m_texture_path)) {
			std::cerr
				<< "Bad asset. Texture \""
				<< m_texture_path.string()
				<< "\" does not exist.";
			exit(EXIT_FAILURE);
		}
		else if (m_texture_path.extension() != ".png") {
			std::cerr
				<< "Bad asset. Texture \""
				<< m_texture_path.string()
				<< "\" needs the file extension .png";
			exit(EXIT_FAILURE);
		}
	}

	loadImageFromDisk();
}
void Texture::stop() noexcept
{
	offloadFromGpu();
	m_image = std::nullopt;
}

void Texture::loadImageFromDisk()
{
	std::vector<uint8_t> encrypted_image;
	lodepng::load_file(encrypted_image, m_texture_path.string());
	m_image = Image{};

	[[maybe_unused]] auto lodepng_error = lodepng::decode(
		m_image.value().data,
		m_image.value().width,
		m_image.value().height,
		encrypted_image
	);

	if constexpr (BuildSettings::mode != BuildSettings::Mode::release)
	{
		if (lodepng_error) {
			std::cerr
				<< "Error decoding image where lodepng was: "
				<< lodepng_error_text(lodepng_error);
			exit(EXIT_FAILURE);
		}
	}
}
