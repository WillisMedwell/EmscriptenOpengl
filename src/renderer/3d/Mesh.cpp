#include "Libraries.hpp"
#include "renderer/3d/Mesh.hpp"
#include "Concept.hpp"

#include <glm/glm.hpp>
#include <wm/Splitters.hpp>

#include <charconv>
#include <fstream>
#include <algorithm>
#include <iterator>
#include "Expected.hpp"

template <MeshType type>
auto loadMeshAs(const std::string& content) -> Expected<std::vector<MeshVariant>, std::string_view>;
template <>
auto loadMeshAs<MeshType::positions_only>(const std::string& content)->Expected<std::vector<MeshVariant>, std::string_view>;
template <>
auto loadMeshAs<MeshType::positions_and_normals>(const std::string& content)->Expected<std::vector<MeshVariant>, std::string_view>;
template <>
auto loadMeshAs<MeshType::positions_normals_uvs>(const std::string& content)->Expected<std::vector<MeshVariant>, std::string_view>;

namespace MeshLoader {
	auto fromObj(std::filesystem::path obj_path) noexcept -> Expected<std::vector<MeshVariant>, std::string_view>
	{
#if BUILD_TARGET == NATIVE_BUILD
		if (!std::filesystem::exists(obj_path)) {
			return { "Specified file cannot be found." };
		}
#endif   
		if (obj_path.extension() != ".obj") {
			return { "The file format must be .obj." };
		}
		std::ifstream infile(obj_path);
		if (!infile.is_open()) {
			return { "Failed to open the file. Probably in use." };
		}

		// preallocate the string size and load all contents.
		infile.seekg(0, std::ios::end);
		size_t fileSize = infile.tellg();
		infile.seekg(0, std::ios::beg);
		std::string content(fileSize, '\0');
		infile.read(&content[0], fileSize);

		std::vector<MeshVariant> meshes;

		/*return loadMeshAs<MeshType::positions_normals_uvs>(content)
			.or_else([&](std::string_view error) {
			return loadMeshAs<MeshType::positions_and_normals>(content);
				})
			.or_else([&](std::string_view error) {
					return loadMeshAs<MeshType::positions_only>(content);
				});*/

		return loadMeshAs<MeshType::positions_normals_uvs>(content);
	}
}

enum class ObjReadingState {
	waiting_for_vertices,
	reading_vertices,
	waiting_for_normals,
	reading_normals,
	waiting_for_uvs,
	reading_uvs,
	waiting_for_indices,
	reading_indices,
	finished,
};

static float toFloat(const Concept::IsContiguousRangeWithUnderlyingType<char> auto& num_string)
{
	float num = 0.0f;
#if BUILD_TARGET == WEB_BUILD
	static std::string tmp;
	tmp.clear();
	std::copy(num_string.begin(), num_string.end(), std::back_inserter(tmp));
	num = std::stof(tmp);
#else 
	std::from_chars(
		num_string.data(), 
		num_string.data() + num_string.size(), 
		num,
		std::chars_format::general
	);
#endif
	return num;
}

template <>
auto loadMeshAs<MeshType::positions_only>(const std::string& content) -> Expected<std::vector<MeshVariant>, std::string_view>
{
	std::vector<glm::vec3> vertices;
	
	vertices.reserve(10000);

	Mesh<MeshType::positions_only> mesh;
	mesh.num_faces = 0;
	std::vector<MeshVariant> meshes;

	ObjReadingState state = ObjReadingState::waiting_for_vertices;

	for (const std::span<const char> line : wm::SplitByElement(content, '\n')) {
		std::span<const char> first_word = *wm::SplitByElement(line, ' ').begin();

		if (state == ObjReadingState::waiting_for_vertices && std::ranges::equal(first_word, std::string_view{ "v" })) {
			state = ObjReadingState::reading_vertices;
		}
		else if (state == ObjReadingState::reading_vertices) {
			if (std::ranges::equal(first_word, std::string_view{ "v" })) {
				state = ObjReadingState::reading_vertices;
			}
			else if (std::ranges::equal(first_word, std::string_view{ "f" })) {
				state = ObjReadingState::reading_indices;
			}
			else {
				state = ObjReadingState::waiting_for_indices;
			}
		}
		else if (state == ObjReadingState::waiting_for_indices && std::ranges::equal(first_word, std::string_view{ "f" })) {
			state = ObjReadingState::reading_indices;
		}
		else if (state == ObjReadingState::reading_indices && !std::ranges::equal(first_word, std::string_view{ "f" })) {
			state = ObjReadingState::waiting_for_vertices;
			Mesh<MeshType::positions_only> other_mesh;
			std::swap(other_mesh, mesh);
			meshes.emplace_back(std::move(other_mesh));
		}

		if (state == ObjReadingState::reading_vertices) {
			glm::vec3 vec3;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec3[0] = toFloat(*begin);
			++begin;
			vec3[1] = toFloat(*begin);
			++begin;
			vec3[2] = toFloat(*begin);
			vertices.emplace_back(std::move(vec3));
		}
		else if (state == ObjReadingState::reading_indices) {
			++mesh.num_faces;
			// [ "x/x/x", "x/x/x", "x/x/x" ]
			auto begin = wm::SplitByElement(line, ' ').begin();
			for (int i = 0; i < 3; i++) {
				++begin;
				// "x/x/x"
				auto indices = *begin;
				// "x"
				auto raw_vertex_index = toFloat(*wm::SplitByElement(indices, '/').begin());
				// the index start with one, or are negative if the file is big enough.
				int32_t vertex_index = (raw_vertex_index > 0) ? raw_vertex_index - 1 : vertices.size() + raw_vertex_index;
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].x);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].y);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].z);
			}
		}
	}

	if (mesh.num_faces != 0 && mesh.vertex_buffer_data.size() != 0) {
		meshes.emplace_back(std::move(mesh));
	}
	return meshes;
}

template <>
auto loadMeshAs<MeshType::positions_and_normals>(const std::string& content) -> Expected<std::vector<MeshVariant>, std::string_view>
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;

	Mesh<MeshType::positions_and_normals> mesh;
	mesh.num_faces = 0;

	std::vector<MeshVariant> meshes;

	ObjReadingState state = ObjReadingState::waiting_for_vertices;

	for (const std::span<const char> line : wm::SplitByElement(content, '\n')) {
		std::span<const char> first_word = *wm::SplitByElement(line, ' ').begin();

		if (state == ObjReadingState::waiting_for_vertices && std::ranges::equal(first_word, std::string_view{ "v" })) {
			state = ObjReadingState::reading_vertices;
		}
		else if (state == ObjReadingState::reading_vertices) {
			if (std::ranges::equal(first_word, std::string_view{ "v" })) {
				state = ObjReadingState::reading_vertices;
			}
			else if (std::ranges::equal(first_word, std::string_view{ "vn" })) {
				state = ObjReadingState::reading_normals;
			}
			else {
				state = ObjReadingState::waiting_for_normals;
			}
		}
		else if (state == ObjReadingState::waiting_for_normals) {
			if (std::ranges::equal(first_word, std::string_view{ "vn" })) {
				state = ObjReadingState::reading_normals;
			}
		}
		else if (state == ObjReadingState::reading_normals) {
			if (std::ranges::equal(first_word, std::string_view{ "vn" })) {
				state = ObjReadingState::reading_normals;
			}
			else if (std::ranges::equal(first_word, std::string_view{ "f" })) {
				state = ObjReadingState::reading_indices;
			}
			else {
				state = ObjReadingState::waiting_for_indices;
			}
		}
		else if (state == ObjReadingState::waiting_for_indices && std::ranges::equal(first_word, std::string_view{ "f" })) {
			state = ObjReadingState::reading_indices;
			if (wm::SplitByElement(line, ' ').evaluate().size() != 4) {
				return { "The obj format is not as expected." };
			}
		}
		else if (state == ObjReadingState::reading_indices && !std::ranges::equal(first_word, std::string_view{ "f" })) {
			state = ObjReadingState::waiting_for_vertices;
			Mesh<MeshType::positions_and_normals> other_mesh;
			std::swap(other_mesh, mesh);
			meshes.emplace_back(std::move(other_mesh));
		}

		if (state == ObjReadingState::reading_vertices) {
			glm::vec3 vec3;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec3[0] = toFloat(*begin);
			++begin;
			vec3[1] = toFloat(*begin);
			++begin;
			vec3[2] = toFloat(*begin);
			vertices.emplace_back(std::move(vec3));
		}
		else if (state == ObjReadingState::reading_normals) {
			glm::vec3 vec3;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec3[0] = toFloat(*begin);
			++begin;
			vec3[1] = toFloat(*begin);
			++begin;
			vec3[2] = toFloat(*begin);
			normals.emplace_back(std::move(vec3));
		}
		else if (state == ObjReadingState::reading_indices) {
			++mesh.num_faces;
			// [ "x/x/x", "x/x/x", "x/x/x" ]
			auto begin = wm::SplitByElement(line, ' ').begin();
			for (int i = 0; i < 3; i++) {
				++begin;
				// "x/x/x"
				auto indices_data = *begin;
				auto indices_begin = wm::SplitByElement(indices_data, '/').begin();

				auto raw_vertex_index = toFloat(*indices_begin);
				auto raw_normal_index = toFloat(*(++(++indices_begin)));

				// the index start with one, or are negative if the file is big enough.
				int32_t vertex_index = (raw_vertex_index > 0) ? raw_vertex_index - 1 : vertices.size() + raw_vertex_index;
				int32_t normal_index = (raw_normal_index > 0) ? raw_normal_index - 1 : normals.size() + raw_normal_index;

				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].x);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].y);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].z);

				mesh.vertex_buffer_data.emplace_back(normals[normal_index].x);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].y);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].z);
			}
		}
	}

	if (mesh.num_faces != 0 && mesh.vertex_buffer_data.size() != 0) {
		meshes.emplace_back(std::move(mesh));
	}
	return meshes;
}

template <>
auto loadMeshAs<MeshType::positions_normals_uvs>(const std::string& content) -> Expected<std::vector<MeshVariant>, std::string_view>
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	std::vector<MeshVariant> meshes;

	Mesh<MeshType::positions_normals_uvs> mesh;
	mesh.num_faces = 0;

	int32_t vertices_index_offset = 0;
	int32_t uvs_index_offset = 0;
	int32_t normals_index_offset = 0;

	ObjReadingState state = ObjReadingState::waiting_for_vertices;
#if 0
	for (const std::span<const char> line : wm::SplitByElement(content, '\n')) {
		std::span<const char> first_word = *wm::SplitByElement(line, ' ').begin();

		if (state == ObjReadingState::waiting_for_vertices && std::ranges::equal(first_word, std::string_view{ "v" })) {
			state = ObjReadingState::reading_vertices;
		}
		else if (state == ObjReadingState::reading_vertices) {
			if (std::ranges::equal(first_word, std::string_view{ "v" })) {
				state = ObjReadingState::reading_vertices;
			}
			else if (std::ranges::equal(first_word, std::string_view{ "vt" })) {
				state = ObjReadingState::reading_uvs;
			}
			else {
				state = ObjReadingState::waiting_for_uvs;
			}
		}
		else if (state == ObjReadingState::waiting_for_uvs) {
			if (std::ranges::equal(first_word, std::string_view{ "vt" })) {
				state = ObjReadingState::reading_uvs;
			}
		}
		else if (state == ObjReadingState::reading_uvs) {
			if (std::ranges::equal(first_word, std::string_view{ "vt" })) {
				state = ObjReadingState::reading_uvs;
			}
			else if (std::ranges::equal(first_word, std::string_view{ "vn" })) {
				state = ObjReadingState::reading_normals;
			}
			else {
				state = ObjReadingState::waiting_for_normals;
			}
		}
		else if (state == ObjReadingState::waiting_for_normals) {
			if (std::ranges::equal(first_word, std::string_view{ "vn" })) {
				state = ObjReadingState::reading_normals;
			}
		}
		else if (state == ObjReadingState::reading_normals) {
			if (std::ranges::equal(first_word, std::string_view{ "vn" })) {
				state = ObjReadingState::reading_normals;
			}
			else if (std::ranges::equal(first_word, std::string_view{ "f" })) {
				state = ObjReadingState::reading_indices;
			}
			else {
				state = ObjReadingState::waiting_for_indices;
			}
		}
		else if (state == ObjReadingState::waiting_for_indices && std::ranges::equal(first_word, std::string_view{ "f" })) {
			state = ObjReadingState::reading_indices;
		}
		else if (state == ObjReadingState::reading_indices && std::ranges::equal(first_word, std::string_view{ "o" })) {
			state = ObjReadingState::waiting_for_vertices;
			Mesh<MeshType::positions_normals_uvs> other_mesh;
			std::swap(other_mesh, mesh);
			meshes.emplace_back(std::move(other_mesh));

			vertices_index_offset += vertices.size();
			normals_index_offset += normals.size();
			uvs_index_offset += uvs.size();

			vertices.clear();
			normals.clear();
			uvs.clear();
		}

		if (state == ObjReadingState::reading_vertices) {
			glm::vec3 vec3;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec3[0] = toFloat(*begin);
			++begin;
			vec3[1] = toFloat(*begin);
			++begin;
			vec3[2] = toFloat(*begin);
			vertices.emplace_back(std::move(vec3));
		}
		else if (state == ObjReadingState::reading_normals) {
			glm::vec3 vec3;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec3[0] = toFloat(*begin);
			++begin;
			vec3[1] = toFloat(*begin);
			++begin;
			vec3[2] = toFloat(*begin);
			normals.emplace_back(std::move(vec3));
		}
		else if (state == ObjReadingState::reading_uvs) {
			glm::vec2 vec2;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec2[0] = toFloat(*begin);
			++begin;
			vec2[1] = toFloat(*begin);
			uvs.emplace_back(std::move(vec2));
		}
		else if (state == ObjReadingState::reading_indices) {
			++mesh.num_faces;
			// [ "x/x/x", "x/x/x", "x/x/x" ]
			auto split = wm::SplitByElement(line, ' ');
			for (auto iter = ++split.begin(); iter != split.end(); ++iter) {
				// "x/x/x"
				auto indices_data = *iter;
				auto indices_begin = wm::SplitByElement(indices_data, '/').begin();


				auto raw_vertex_index = toFloat(*indices_begin);
				auto raw_uv_index = toFloat(*(++indices_begin));
				auto raw_normal_index = toFloat(*(++indices_begin));

				// the index start with one, or are negative if the file is big enough.
				int32_t vertex_index = (raw_vertex_index > 0) ? raw_vertex_index - 1 - vertices_index_offset : vertices.size() + raw_vertex_index;
				int32_t normal_index = (raw_normal_index > 0) ? raw_normal_index - 1 - normals_index_offset : normals.size() + raw_normal_index;
				int32_t uv_index = (raw_uv_index > 0) ? raw_uv_index - 1 - uvs_index_offset : uvs.size() + raw_uv_index;

				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].x);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].y);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].z);

				mesh.vertex_buffer_data.emplace_back(normals[normal_index].x);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].y);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].z);

				mesh.vertex_buffer_data.emplace_back(uvs[uv_index].x);
				mesh.vertex_buffer_data.emplace_back(uvs[uv_index].y);
			}
		}
	}
#else 

	for (const std::span<const char> line : wm::SplitByElement(content, '\n'))
	{
		auto word_range = wm::SplitByElement(line, ' ');
		auto first_word = std::string_view{ *word_range.begin() };
		if (first_word == "o")
		{
			if (mesh.num_faces != 0 && mesh.vertex_buffer_data.size() != 0) {
				meshes.emplace_back(std::move(mesh));
			}
		}
		else if (first_word == "v")
		{
			glm::vec3 vec3;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec3[0] = toFloat(*begin);
			++begin;
			vec3[1] = toFloat(*begin);
			++begin;
			vec3[2] = toFloat(*begin);
			vertices.emplace_back(std::move(vec3));
		}
		else if (first_word == "vt")
		{
			glm::vec2 vec2;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec2[0] = toFloat(*begin);
			++begin;
			vec2[1] = toFloat(*begin);
			uvs.emplace_back(std::move(vec2));
		}
		else if (first_word == "vn")
		{
			glm::vec3 vec3;
			auto begin = ++(wm::SplitByElement(line, ' ').begin());
			vec3[0] = toFloat(*begin);
			++begin;
			vec3[1] = toFloat(*begin);
			++begin;
			vec3[2] = toFloat(*begin);
			normals.emplace_back(std::move(vec3));
		}
		else if (first_word == "f")
		{
			++mesh.num_faces;
			// [ "x/x/x", "x/x/x", "x/x/x" ]
#if 1
			auto begin = wm::SplitByElement(line, ' ').begin();
			for (int i = 0; i < 3; i++) {
				++begin;
				// "x/x/x"
				auto indices_data = *begin;
				auto indices_begin = wm::SplitByElement(indices_data, '/').begin();

				auto raw_vertex_index = toFloat(*indices_begin);
				auto raw_uv_index = toFloat(*(++indices_begin));
				auto raw_normal_index = toFloat(*(++indices_begin));

				// the index start with one, or are negative if the file is big enough.
				int32_t vertex_index = (raw_vertex_index > 0) ? raw_vertex_index - 1 : vertices.size() + raw_vertex_index;
				int32_t normal_index = (raw_normal_index > 0) ? raw_normal_index - 1 : normals.size() + raw_normal_index;
				int32_t uv_index = (raw_normal_index > 0) ? raw_uv_index - 1 : uvs.size() + raw_uv_index;


				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].x);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].y);
				mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].z);

				mesh.vertex_buffer_data.emplace_back(normals[normal_index].x);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].y);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].z);

				mesh.vertex_buffer_data.emplace_back(uvs[uv_index].x);
				mesh.vertex_buffer_data.emplace_back(uvs[uv_index].y);
			}
#else 
			auto begin = wm::SplitByElement(line, ' ').begin();
			for (int i = 0; i < 3; i++) {
				++begin;
				// "x/x/x"
				auto indices_data = *begin;
				auto indices_begin = wm::SplitByElement(indices_data, '/').begin();

				int num_count = 0;
				for (auto num : wm::SplitByElement(indices_data, '/'))
				{
					++num_count;
					if (num_count == 0)
					{
						float raw_vertex_index = toFloat(num);
						int32_t vertex_index = (raw_vertex_index > 0) ? raw_vertex_index - 1 : vertices.size() + raw_vertex_index;
						mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].x);
						mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].y);
						mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].z);

					}
					else if (num_count == 1)
					{
						float raw_uv_index = toFloat(num);
						int32_t vertex_index = (raw_vertex_index > 0) ? raw_vertex_index - 1 : vertices.size() + raw_vertex_index;
						mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].x);
						mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].y);
						mesh.vertex_buffer_data.emplace_back(vertices[vertex_index].z);

					}
					else if (num_count == 2)
					{

					}
				}

				auto raw_uv_index = toFloat(*(++indices_begin));
				auto raw_normal_index = toFloat(*(++indices_begin));

				int32_t normal_index = (raw_normal_index > 0) ? raw_normal_index - 1 : normals.size() + raw_normal_index;
				int32_t uv_index = (raw_normal_index > 0) ? raw_uv_index - 1 : uvs.size() + raw_uv_index;

				mesh.vertex_buffer_data.emplace_back(normals[normal_index].x);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].y);
				mesh.vertex_buffer_data.emplace_back(normals[normal_index].z);

				mesh.vertex_buffer_data.emplace_back(uvs[uv_index].x);
				mesh.vertex_buffer_data.emplace_back(uvs[uv_index].y);
			}
#endif 
		}
	}
#endif
	if (mesh.num_faces != 0 && mesh.vertex_buffer_data.size() != 0) {
		meshes.emplace_back(std::move(mesh));
	}
	return meshes;
}

