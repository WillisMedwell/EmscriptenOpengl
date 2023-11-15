#include "renderer/3d/MeshRenderer.hpp"
#include <array>

bool SharedIndexBuffer::hasCapacityFor(size_t num_faces)
{
	return SharedIndexBuffer::index_buffer_data.size() >= num_faces * 3;
}

void SharedIndexBuffer::makeCapacityFor(size_t num_faces)
{
	if (!hasCapacityFor(num_faces)) {
		index_buffer_data.resize(num_faces * 3);
		for (uint32_t i = 0; i < num_faces; ++i) {
			index_buffer_data[(i * 3) + 0] = 0 + (i * 3);
			index_buffer_data[(i * 3) + 1] = 1 + (i * 3);
			index_buffer_data[(i * 3) + 2] = 2 + (i * 3);
		}
	}
	ib.bind();
	ib.loadIndices(index_buffer_data);
}

std::vector<uint32_t> SharedIndexBuffer::index_buffer_data;
IndexBuffer SharedIndexBuffer::ib;

void MeshRenderer<MeshType::positions_normals_uvs>::bindAll()
{
	m_va.bind();
	m_vb.bind();
}

void MeshRenderer<MeshType::positions_normals_uvs>::unbindAll()
{
	m_va.unbind();
	m_vb.unbind();
}

void MeshRenderer<MeshType::positions_normals_uvs>::init()
{
	SharedIndexBuffer::ib.init();
	m_vb.init();
	m_va.init();
	auto layout = getLayout();
	m_va.attachBufferAndLayout(m_vb, layout);
	m_va.unbind();
	m_vb.unbind();
}

void MeshRenderer<MeshType::positions_normals_uvs>::stop()
{
	m_vb.stop();
	m_va.stop();
}

void MeshRenderer<MeshType::positions_normals_uvs>::draw(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const Mesh<MeshType::positions_normals_uvs>& mesh, Shader& shader)
{
	bindAll();
	//shader.bind();
	SharedIndexBuffer::ib.bind();
	m_vb.loadVertices(mesh.vertex_buffer_data);
	SharedIndexBuffer::makeCapacityFor(mesh.num_faces);

	auto printAndQuit = [&](std::string_view msg) -> std::string_view {
		std::cerr
			<< "Failed to find uniform where the key searched was: "
			<< msg << '\n';
		exit(EXIT_FAILURE);
		return {};
		};

	shader.setUniform("u_model_matrix", model).OnError(printAndQuit);
	shader.setUniform("u_view_matrix", view).OnError(printAndQuit);
	shader.setUniform("u_projection_matrix", projection).OnError(printAndQuit);

	glDrawElements(GL_TRIANGLES, mesh.num_faces * 3, GL_UNSIGNED_INT, (void*)0);

	shader.unbind();
	unbindAll();
}

void MeshRenderer<MeshType::positions_normals_uvs>::draw(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const Mesh<MeshType::positions_normals_uvs>& mesh, Shader& shader, const std::vector<PointLight>& lights)
{
	bindAll();
	//shader.bind();
	SharedIndexBuffer::ib.bind();
	m_vb.loadVertices(mesh.vertex_buffer_data);
	SharedIndexBuffer::makeCapacityFor(mesh.num_faces);

	auto printAndQuit = [&](std::string_view msg) -> std::string_view {
		std::cerr
			<< "Failed to find uniform where the key searched was: "
			<< msg << '\n';
		exit(EXIT_FAILURE);
		return {};
		};

	shader.setUniform("u_model_matrix", model).OnError(printAndQuit);
	shader.setUniform("u_view_matrix", view).OnError(printAndQuit);
	shader.setUniform("u_projection_matrix", projection).OnError(printAndQuit);

	// single digit verison
	constexpr static auto sd_key_pos = std::string_view{ "u_point_lights[$].position" };
	constexpr static auto sd_key_colour = std::string_view{ "u_point_lights[$].colour" };
	constexpr static auto sd_key_intensity = std::string_view{ "u_point_lights[$].intensity" };
	constexpr static auto sd_key_constant = std::string_view{ "u_point_lights[$].constant" };
	constexpr static auto sd_key_linear = std::string_view{ "u_point_lights[$].linear" };
	constexpr static auto sd_key_quadratic = std::string_view{ "u_point_lights[$].quadratic" };
	constexpr static auto sd_key_ambient = std::string_view{ "u_point_lights[$].ambient_coefficient" };
	constexpr static auto sd_key_specular = std::string_view{ "u_point_lights[$].specular_exponent" };
	constexpr static auto sd_replacement_index = std::distance(sd_key_pos.begin(), std::ranges::find(sd_key_pos, '$'));


	// not my greatest bit of code but it works i suppose.
	std::array<char, 300> buffer{};
	std::copy(sd_key_pos.begin(), sd_key_pos.end(), buffer.begin());
	buffer[sd_key_pos.size()] = '\0';

	size_t index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.position).OnError(printAndQuit);
		++index;
	}

	std::copy(sd_key_colour.begin() + sd_replacement_index, sd_key_colour.end(), buffer.begin() + sd_replacement_index);
	buffer[sd_key_colour.size()] = '\0';
	index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.colour).OnError(printAndQuit);
		++index;
	}

	std::copy(sd_key_intensity.begin() + sd_replacement_index, sd_key_intensity.end(), buffer.begin() + sd_replacement_index);
	buffer[sd_key_intensity.size()] = '\0';
	index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.intensity).OnError(printAndQuit);
		++index;
	}

	std::copy(sd_key_linear.begin() + sd_replacement_index, sd_key_linear.end(), buffer.begin() + sd_replacement_index);
	buffer[sd_key_linear.size()] = '\0';
	index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.attenuation.constant).OnError(printAndQuit);
		++index;
	}

	std::copy(sd_key_quadratic.begin() + sd_replacement_index, sd_key_quadratic.end(), buffer.begin() + sd_replacement_index);
	buffer[sd_key_quadratic.size()] = '\0';
	index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.attenuation.quadratic).OnError(printAndQuit);
		++index;
	}

	std::copy(sd_key_ambient.begin() + sd_replacement_index, sd_key_ambient.end(), buffer.begin() + sd_replacement_index);
	buffer[sd_key_ambient.size()] = '\0';
	index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.ambient_coefficient.value()).OnError(printAndQuit);
		++index;
	}

	std::copy(sd_key_specular.begin() + sd_replacement_index, sd_key_specular.end(), buffer.begin() + sd_replacement_index);
	buffer[sd_key_specular.size()] = '\0';
	index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.specular_exponent.value()).OnError(printAndQuit);
		++index;
	}

	std::copy(sd_key_constant.begin() + sd_replacement_index, sd_key_constant.end(), buffer.begin() + sd_replacement_index);
	buffer[sd_key_constant.size()] = '\0';
	index = 0;
	for (auto light : lights | std::views::take(10)) {
		buffer[sd_replacement_index] = static_cast<char>('0' + index);
		shader.setUniform(buffer.data(), light.intensity).OnError(printAndQuit);
		++index;
	}

	shader.setUniform("u_point_lights_size", static_cast<float>(std::min(lights.size(), size_t{ 10 })));

	glDrawElements(GL_TRIANGLES, mesh.num_faces * 3, GL_UNSIGNED_INT, (void*)0);

	unbindAll();
}