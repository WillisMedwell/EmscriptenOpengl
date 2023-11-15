#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glaze/glaze.hpp>
#include <cmath>
#include <iostream>

class Camera {
public:
	glm::vec3 camera_pos = { 0, 2, 0 };
	glm::vec3 camera_vel = { 0, 0, 0 };
	glm::vec3 camera_dir = { 0, 0.5, 0 };

	//float yaw = 0.0f;
	//float pitch = 0.0f;

	// settings
	constexpr static float z_near = 0.01f;
	constexpr static float z_far = 1000.0f;
	constexpr static float fov = glm::radians(45.0f);
	constexpr static glm::vec3 camera_up = { 0, 1.0f, 0 };

	constexpr static float max_velocity_magnitude = 25.0f;
	constexpr static float mouse_sensitivity = 10.0f;

	friend struct glz::meta<Camera>;
public:
	auto update(float dt, const Input& input)
	{
		// update camera position
		{
			glm::vec3 camera_vel = { 0, 0, 0 };

			auto vel = camera_dir - camera_pos;
			vel.y = 0;

			if (input.isKeyDown('W') && !input.isKeyDown('S')) {
				camera_vel += glm::normalize(vel);
			}
			else if (!input.isKeyDown('W') && input.isKeyDown('S')) {
				camera_vel -= glm::normalize(vel);
			}

			if (input.isKeyDown('A') && !input.isKeyDown('D')) {
				camera_vel -= glm::normalize(glm::cross(vel, camera_up));
			}
			else if (!input.isKeyDown('A') && input.isKeyDown('D')) {
				camera_vel += glm::normalize(glm::cross(vel, camera_up));
			}

			if(input.isKeyDown(Input::Key::left_shift) && !input.isKeyDown(Input::Key::space)) {
				camera_vel -= glm::vec3(0,1,0);
			}
			else if(!input.isKeyDown(Input::Key::left_shift) && input.isKeyDown(Input::Key::space)) {
				camera_vel += glm::vec3(0,1,0);
			}

			if (glm::length(camera_vel) > 1e-5) {  // Check length against small epsilon
				camera_vel = glm::normalize(camera_vel) * max_velocity_magnitude;
			}
			camera_pos += camera_vel * dt;
		}

		// update camera direction
		{
			auto offset = input.getMouseOffset() * mouse_sensitivity;

			auto getYaw = [&](const glm::vec3& vec) {
				return glm::degrees(atan2(vec.z, vec.x)); // Use vec.x instead of vec.y
				};
			auto getPitch = [&](const glm::vec3& vec) {
				float length = glm::length(vec);
				float pitch = asinf(vec.y / length);
				pitch = glm::degrees(pitch);
				return pitch;
			};

			float yaw = getYaw(camera_dir);
			float pitch = getPitch(camera_dir);

			pitch = std::max(std::min(pitch, 89.0f), -89.0f);

			camera_dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			camera_dir.y = sin(glm::radians(pitch));
			camera_dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			camera_dir = glm::normalize(camera_dir);
		}
	}

	inline auto getProjectionMatrix(float width, float height) const noexcept -> glm::mat4
	{
		return glm::perspective(fov, width / height, z_near, z_far);
	}
	inline auto getViewMatrix() const noexcept -> glm::mat4
	{
		//return glm::lookAt(camera_pos, camera_pos + camera_dir, camera_up);
		return glm::lookAt(camera_pos, camera_dir, camera_up);
	}
};

template <>
struct glz::meta<Camera> {
	using T = Camera;
	static constexpr auto value = object(
		"Position", &T::camera_pos,
		"Velocity", &T::camera_vel,
		"Direction", &T::camera_dir
	);
};
