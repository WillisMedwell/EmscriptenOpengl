#pragma once

#include "renderer/3d/Camera.hpp"
#include "renderer/3d/Light.hpp"
#include "renderer/3d/Mesh.hpp"
#include "renderer/3d/MeshRenderer.hpp"

#include "Components.hpp"
#include "Ecs.hpp"
#include "renderer/core/Input.hpp"
#include "renderer/core/Shader.hpp"

#include <glaze/glaze.hpp>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

namespace SceneTypes
{
	using ShaderKey = std::tuple<std::string, std::string, std::optional<std::string>>;
	using MeshKey = std::string;
	using TextureKey = std::string;
} 	

struct SerialisedEntity;

struct Model {
	struct Transforms {
		glm::vec3 translation;
		float scale;
	};

	struct ModelPart {
		SceneTypes::MeshKey mesh_key;
		SceneTypes::ShaderKey shader_key;
		Transforms transforms;
		bool needs_point_lights;
		std::optional<SceneTypes::TextureKey> texture_key;
	};

	std::vector<ModelPart> model_parts;
};

class Scene {
public: // Types

private:
	struct ShaderKeyHash {
		std::size_t operator()(const SceneTypes::ShaderKey& key) const
		{
			const auto& [part1, part2, optPart3] = key;
			std::size_t h1 = std::hash<std::string>{}(part1);
			std::size_t h2 = std::hash<std::string>{}(part2);
			std::size_t h3 = optPart3 ? std::hash<std::string> {}(*optPart3) : 0;
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};

public: // members
	glm::u8vec3 background_colour;
	Camera camera;
	std::vector<Model> models;
	std::vector<PointLight> point_lights;

	SparseFlexEcs<Model, Camera, PointLight, DirectionalLight> entities;

private:
	std::unordered_map<SceneTypes::MeshKey, std::vector<MeshVariant>> m_mesh_lookup;
	std::unordered_map<SceneTypes::ShaderKey, Shader, ShaderKeyHash> m_shader_lookup;
	std::unordered_map<SceneTypes::TextureKey, Texture> m_texture_lookup;

private: // methods
	[[nodiscard]] auto loadResources() -> Expected<void, std::string_view>;
	auto offloadResources() -> void;

public:
	Scene() = default;

	auto reload()
	{
		offloadResources();
		return loadResources();
	}

	[[nodiscard]] auto init(std::filesystem::path scene_path) -> Expected<void, std::string_view>
	{
		stop();

		if (scene_path.extension() != ".json") {
			return { "Scene resource must be .json." };
		}
		if (!std::filesystem::exists(scene_path)) {
			return { "Scene resource cannot be found." };
		}
		std::ifstream infile(scene_path);
		if (!infile.is_open()) {
			return { "Unable to open the scene file." };
		}

		// preallocate the string size and load all contents.
		infile.seekg(0, std::ios::end);
		size_t fileSize = infile.tellg();
		infile.seekg(0, std::ios::beg);
		std::string content(fileSize, '\0');
		infile.read(&content[0], fileSize);

		if (auto has_result = glz::read_json<Scene>(content); has_result) {
			has_result.value().camera = camera;
			*this = has_result.value();
		}
		else {
			std::cerr << glz::format_error(has_result.error(), content) << '\n';
			return { "Failed parsing scene .json file" };
		}
		return loadResources();
	}
	auto stop() -> void
	{
		offloadResources();
		background_colour = { 0, 0, 0 };
		models.clear();
		point_lights.clear();
	}

	auto update(float dt, const Input& input)
	{
		camera.update(dt, input);
	}

	friend class Renderer;
	friend class MeshRenderer<MeshType::positions_normals_uvs>;
	friend struct Model;
	friend struct glz::meta<Scene>;
};

inline auto Scene::loadResources() -> Expected<void, std::string_view>
{
	for (Model& model : models) {
		for (const auto& [mesh_key, shader_key, transforms, needs_point_lights, maybe_texture_key] : model.model_parts) {

			auto addLoadedMeshes = [&](auto&& meshes) -> Expected<std::vector<MeshVariant>, std::string_view> {
				m_mesh_lookup[mesh_key] = meshes;
				return {};
				};

			auto printError = [&](std::string_view error) -> Expected<std::vector<MeshVariant>, std::string_view> {
				if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
					std::cerr
						<< "Unable to load mesh. Where the error was:"
						<< error
						<< '\n'
						<< std::endl;
					exit(EXIT_FAILURE);
				}
				return { error };
				};

			if (!m_mesh_lookup.contains(mesh_key)) {
				MeshLoader::fromObj(mesh_key)
					.OnValue(addLoadedMeshes)
					.OnError(printError);
			}

			if (!m_shader_lookup.contains(shader_key)) {
				auto [vert, frag, maybe_geo] = shader_key;
				auto& shader = (m_shader_lookup[shader_key] = Shader());
				shader.init(vert, frag, maybe_geo);
				shader.uploadToGpu();
			}

			if (maybe_texture_key && !m_texture_lookup.contains(maybe_texture_key.value())) {
				Texture& texture = (m_texture_lookup[maybe_texture_key.value()] = Texture{});
				texture.init(maybe_texture_key.value());
				texture.uploadToGpu();
			}
		}
	}

	Model model;
	model.model_parts.emplace_back(Model::ModelPart{
		.mesh_key = "assets/objects/trail.obj",
		.shader_key = {},
		.transforms = {.translation = { 0, 0, 0 }, .scale = 1.0f },
		.needs_point_lights = false,
		.texture_key = std::nullopt
		});

	for (auto [model] : entities.forAnyWith<Model>()) {
		for (const auto& [mesh_key, shader_key, transforms, needs_point_lights, maybe_texture_key] : model.model_parts) {

			auto addLoadedMeshes = [&](auto&& meshes) -> Expected<std::vector<MeshVariant>, std::string_view> {
				m_mesh_lookup[mesh_key] = meshes;
				return {};
				};

			auto printError = [&](std::string_view error) -> Expected<std::vector<MeshVariant>, std::string_view> {
				if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
					std::cerr
						<< "Unable to load mesh. Where the error was:"
						<< error
						<< '\n'
						<< std::endl;
					exit(EXIT_FAILURE);
				}
				return { error };
				};

			if (!m_mesh_lookup.contains(mesh_key)) {
				MeshLoader::fromObj(mesh_key)
					.OnValue(addLoadedMeshes)
					.OnError(printError);
			}
		}
	}
	return {};
}
inline auto Scene::offloadResources() -> void
{
	m_mesh_lookup.clear();
	m_shader_lookup.clear();
	m_texture_lookup.clear();
}

template <>
struct glz::meta<glm::u8vec3> {
	using T = glm::u8vec3;
	static constexpr auto value = object(
		"r", &T::r,
		"g", &T::g,
		"b", &T::b);
};

template <>
struct glz::meta<glm::vec3> {
	using T = glm::vec3;
	static constexpr auto value = object(
		"x", &T::x,
		"y", &T::y,
		"z", &T::z);
};

template <>
struct glz::meta<Model::Transforms> {
	using T = Model::Transforms;
	static constexpr auto value = object(
		"Translation", &T::translation,
		"Scale", &T::scale);
};

template <>
struct glz::meta<Model::ModelPart> {
	using T = Model::ModelPart;
	static constexpr auto value = object(
		"Mesh-File", &T::mesh_key,
		"Shader-Files", &T::shader_key,
		"Transforms", &T::transforms,
		"Needs-Point-Lights", &T::needs_point_lights,
		"Texture-File", &T::texture_key);
};

template <>
struct glz::meta<Model> {
	using T = Model;
	static constexpr auto value = object(
		"Model-Parts", &T::model_parts);
};

template <>
struct glz::meta<Scene> {
	using T = Scene;
	static constexpr auto value = object(
		"Background-Colour", &T::background_colour,
		"Models", &T::models,
		"Camera", &T::camera,
		"Point-Lights", &T::point_lights);
};