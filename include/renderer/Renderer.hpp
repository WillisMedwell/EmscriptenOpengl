#pragma once

#include <array>
#include <tuple>

#include "3d/MeshRenderer.hpp"
#include "3d/Scene.hpp"
#include "core/OpenglContext.hpp"
#include "3d/Camera.hpp"

class Renderer {
	MeshRenderer<MeshType::positions_normals_uvs> m_pnu_renderer;
public:
	void init()
	{
		m_pnu_renderer.init();

	}
	void stop()
	{
		m_pnu_renderer.stop();
	}
	void draw(Scene& scene, float width, float height)
	{
		auto isPNU = [&](MeshVariant& mesh) {
			return std::holds_alternative<Mesh<MeshType::positions_normals_uvs>>(mesh);
			};
		auto asPNU = [](MeshVariant& mesh) -> Mesh<MeshType::positions_normals_uvs>&{
			return std::get<Mesh<MeshType::positions_normals_uvs>>(mesh);
			};

		auto getModelMatrx = [&](const Model::Transforms& transformation) {
			auto I = glm::mat4(1.0f);

			auto A = glm::translate(I, transformation.translation);
			return glm::scale(A, glm::vec3(transformation.scale));
			};

		auto view = scene.camera.getViewMatrix();
		auto proj = scene.camera.getProjectionMatrix(width, height);

		for (Model& model : scene.models) {
			for (const auto& [mesh_key, shader_key, transforms, has_point_lighting, maybe_texture_key] : model.model_parts) {
				auto& meshes = scene.m_mesh_lookup[mesh_key];
				auto& shader = scene.m_shader_lookup[shader_key];

				auto model_matrix = getModelMatrx(transforms);
				shader.bind();

				if (!has_point_lighting && !maybe_texture_key) {
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader);
					}
				}
				else if (has_point_lighting) {
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader, scene.point_lights);
					}
				}
				else if (maybe_texture_key) {
					scene.m_texture_lookup[maybe_texture_key.value()].bind(2);
					shader.setUniform("u_texture", int32_t{ 2 });
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader);
					}
				}
			}
		}

		for (auto [model] : scene.entities.forAnyWith<Model>()) {
			for (const auto& [mesh_key, shader_key, transforms, has_point_lighting, maybe_texture_key] : model.model_parts) {
				auto& meshes = scene.m_mesh_lookup[mesh_key];
				auto& shader = scene.m_shader_lookup[shader_key];

				auto model_matrix = getModelMatrx(transforms);
				shader.bind();

				if (!has_point_lighting && !maybe_texture_key) {
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader);
					}
				}
				else if (has_point_lighting) {
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader, scene.point_lights);
					}
				}
				else if (maybe_texture_key) {
					scene.m_texture_lookup[maybe_texture_key.value()].bind(2);
					shader.setUniform("u_texture", int32_t{ 2 });
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader);
					}
				}
			}
		}
	}
	void drawShadows(Scene& scene, float width, float height)
	{
		Camera shadow_camera;
		shadow_camera.camera_pos = { -6.13285,10.4158,5.33445 };
		shadow_camera.camera_dir = { 0.0174524,0.999848,0 };

		auto isPNU = [&](MeshVariant& mesh) {
			return std::holds_alternative<Mesh<MeshType::positions_normals_uvs>>(mesh);
			};
		auto asPNU = [](MeshVariant& mesh) -> Mesh<MeshType::positions_normals_uvs>&{
			return std::get<Mesh<MeshType::positions_normals_uvs>>(mesh);
			};

		auto getModelMatrx = [&](const Model::Transforms& transformation) {
			auto I = glm::mat4(1.0f);

			auto A = glm::translate(I, transformation.translation);
			return glm::scale(A, glm::vec3(transformation.scale));
			};

		auto view = shadow_camera.getViewMatrix();
		auto proj = shadow_camera.getProjectionMatrix(width, height);

		for (Model& model : scene.models) {
			for (const auto& [mesh_key, shader_key, transforms, has_point_lighting, maybe_texture_key] : model.model_parts) {
				auto& meshes = scene.m_mesh_lookup[mesh_key];
				auto& shader = scene.m_shader_lookup[shader_key];

				auto model_matrix = getModelMatrx(transforms);
				shader.bind();

				if (!has_point_lighting && !maybe_texture_key) {
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader);
					}
				}
				else if (has_point_lighting) {
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader, scene.point_lights);
					}
				}
				else if (maybe_texture_key) {
					scene.m_texture_lookup[maybe_texture_key.value()].bind(2);
					shader.setUniform("u_texture", int32_t{ 2 });
					for (const auto& mesh : meshes | std::views::filter(isPNU) | std::views::transform(asPNU)) {
						m_pnu_renderer.draw(model_matrix, view, proj, mesh, shader);
					}
				}
			}
		}
	}
};