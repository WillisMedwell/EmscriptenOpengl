#pragma once

#include "Libraries.hpp"

#include <filesystem>
#include <optional>
#include <cstdint>
#include <vector>

class Texture {
	struct Image {
		std::vector<uint8_t> data;
		uint32_t width;
		uint32_t height;
	};
	std::filesystem::path m_texture_path;

	std::optional<Image> m_image;
	std::optional<uint32_t> m_texture_id;
	std::optional<uint32_t> m_bound_slot;

public:
	void uploadToGpu() noexcept;
	void offloadFromGpu() noexcept;

	void bind(uint32_t slot) noexcept;
	void unbind() noexcept;

	void init(std::filesystem::path texture_path) noexcept;
	void reload() noexcept;
	void stop() noexcept;

	Texture();
	//Texture(const Texture&) = delete;
	//Texture(Texture&&) noexcept;

	~Texture() { stop(); }
private:
	void loadImageFromDisk();
};