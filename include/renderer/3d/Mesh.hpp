#pragma once

#include <filesystem>
#include <string_view>
#include <variant>
#include <vector>
#include "Expected.hpp"

enum class MeshType {
    positions_only = 3,
    positions_and_normals = 6,
    positions_normals_uvs = 8
};

template <MeshType Type>
struct Mesh {
};

template <>
struct Mesh<MeshType::positions_only> {
    constexpr static size_t floats_per_vertex_attribute = 3;
    size_t num_faces;
    std::vector<float> vertex_buffer_data;
};
template <>
struct Mesh<MeshType::positions_and_normals> {
    constexpr static size_t floats_per_vertex_attribute = 3 + 3;
    size_t num_faces;
    std::vector<float> vertex_buffer_data;
};
template <>
struct Mesh<MeshType::positions_normals_uvs> {
    constexpr static size_t floats_per_vertex_attribute = 3 + 3 + 2;
    size_t num_faces;
    std::vector<float> vertex_buffer_data;
    void* texture;
};

using MeshVariant = std::variant<
    Mesh<MeshType::positions_only>,
    Mesh<MeshType::positions_and_normals>,
    Mesh<MeshType::positions_normals_uvs>>;

namespace MeshLoader {
    auto fromObj(std::filesystem::path obj_path) noexcept -> Expected<std::vector<MeshVariant>, std::string_view>;
}
