#pragma once

#include "Ecs.hpp"
#include "renderer/3d/Scene.hpp"

#include <glaze/core/macros.hpp>
#include <glaze/glaze.hpp>

struct SerialisedEntity {
	using Components = std::tuple<std::optional<Model>, std::optional<Camera>, std::optional<PointLight>, std::optional<DirectionalLight>>;

	Components components;

	template <typename Predicate, size_t I = 0>
	auto forEach(Predicate predicate)
	{
		if constexpr (I == std::tuple_size_v<Components>) {
			return;
		}
		else {
			using OptComponent = std::tuple_element_t<I, Components>;
			OptComponent& opt_component = std::get<I>(components);
			if (opt_component.has_value()) {
				predicate(opt_component.value());
			}
			return forEach<Predicate, I + 1>(predicate);
		}
	}

	template <typename... Args>
	void addToEcs(SparseFlexEcs<Args...>* ecs)
	{
		auto e = ecs->createEntity();

		auto componentToEntity = [&](auto& component) {
			using Component = std::remove_reference_t<decltype(component)>;
			ecs->template addComponentToEntity<Component>(e, component);
			};
		forEach(componentToEntity);
	}

	template <typename T>
	void addComponent(T t)
	{
		std::get<std::optional<T>>(components) = t;
	}
};

template <>
struct glz::meta<SerialisedEntity> {
	using T = SerialisedEntity;
	static constexpr auto value = object(
		"Entity's-Components", &T::components);
};

struct SerialisedScene {
	std::vector<SerialisedEntity> entities;

	void loadIntoScene(Scene* scene)
	{
		scene->stop();
		
		for (SerialisedEntity& entity : entities) {
			entity.addToEcs(&scene->entities);
		}
		scene->reload();
			
			/*.OnError([&](auto error) {
			std::cerr << error << '\n';
			exit(EXIT_FAILURE);
			return error;
		});*/
	}

	auto load(std::filesystem::path scene_path) -> Expected<void, std::string_view>
	{
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

		if (auto has_result = glz::read_json<SerialisedScene>(content); has_result) {
			*this = has_result.value();
			return {};
		}
		else {
			std::cerr << glz::format_error(has_result.error(), content) << '\n';
			return { "Failed parsing scene .json file" };
		}
	}
};

template <>
struct glz::meta<SerialisedScene> {
	using T = SerialisedScene;
	static constexpr auto value = object(
		"Entities", &T::entities);
};