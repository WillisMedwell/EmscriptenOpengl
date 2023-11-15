#pragma once

#include "Libraries.hpp"

#include <cstdint>
#include <iostream>
#include <optional>

#include "renderer/core/IndexBuffer.hpp"
#include "renderer/core/Shader.hpp"
#include "renderer/core/VertexArray.hpp"
#include "renderer/core/VertexBuffer.hpp"
#include "renderer/core/VertexBufferLayout.hpp"

#include "BuildSettings.hpp"

class FrameBuffer;
class ScreenFrameBuffer;

static Shader* bound_shader = nullptr;

class FrameBuffer {
	std::optional<uint32_t> m_fb;

public:
	std::optional<uint32_t> m_colour_attachment;
	std::optional<uint32_t> m_depth_stencil_attachment;

private:
	uint32_t m_width, m_height;

	static std::optional<VertexBuffer> m_vb;
	static std::optional<VertexArray> m_va;
	std::optional<Shader> m_s;

	constexpr static auto getLayout() -> VertexBufferLayout
	{
		VertexBufferLayout vbl;
		vbl.push<float>(2);
		vbl.push<float>(2);
		return vbl;
	}

public:
	void init(uint32_t width, uint32_t height);

	void stop();
	void bind();
	void unbind();

	void draw(FrameBuffer& fb, std::string_view frag_shader_path)
	{
		bind();
		if (!m_vb.has_value() || !m_va.has_value()) {
			m_vb = VertexBuffer{};
			m_va = VertexArray{};

			m_vb.value().init();
			m_va.value().init();
			auto layout = getLayout();
			m_va.value().attachBufferAndLayout(m_vb.value(), layout);
			m_vb.value().unbind();
			m_va.value().unbind();
		}
		if (!m_s.has_value()) {
			m_s = Shader{};
			m_s.value().init("assets/shaders/screen.vert.glsl", frag_shader_path);
			m_s.value().uploadToGpu();
			m_s.value().unbind();
		}

		m_vb.value().bind();
		m_va.value().bind();
		m_s.value().bind();
		static float u_time = 1.0f;

		u_time = static_cast<int>(u_time++) % 100 + 1;

		m_s.value().setUniform("u_time", u_time);

		bound_shader = &m_s.value();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fb.m_colour_attachment.value());


		constexpr static auto vertex_buffer_data = std::to_array({ -1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f });

		m_s.value().setUniform("u_screen_texture", 0);
		m_vb.value().loadVertices(vertex_buffer_data);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		m_va.value().unbind();
		m_vb.value().unbind();
		m_s.value().unbind();

		this->unbind();
	}

	void clearBuffer();

	~FrameBuffer() { stop(); }

	auto shader() -> std::optional<Shader>&
	{
		return m_s;
	}

	friend class ScreenFrameBuffer;
};

inline void FrameBuffer::clearBuffer()
{
	this->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
inline void FrameBuffer::init(uint32_t width, uint32_t height)
{
	stop();
	m_width = width;
	m_height = height;

	{ // generate and bind, check for error.
		m_fb = 0;
		glGenFramebuffers(1, &m_fb.value());
		m_fb = (m_fb.value() == 0) ? std::nullopt : m_fb;
		bind();
	}

	{ // geneate texture/colour attachment.
		m_colour_attachment = 0;
		glGenTextures(1, &m_colour_attachment.value());
		glBindTexture(GL_TEXTURE_2D, m_colour_attachment.value());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour_attachment.value(), 0);

	}

	{ // create depth stencil attachment.
		m_depth_stencil_attachment = 0;
		glGenRenderbuffers(1, &m_depth_stencil_attachment.value());
		glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_attachment.value());
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_stencil_attachment.value());
	}


	{ // validate the creation
		if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
			bind();
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Failed to init framebuffer.\n";
				assert(false);
				m_fb = std::nullopt;
				m_colour_attachment = std::nullopt;
				m_depth_stencil_attachment = std::nullopt;
			}
		}
	}
}
inline void FrameBuffer::stop()
{
	if (m_colour_attachment) {
		glDeleteTextures(1, &m_colour_attachment.value());
	}
	if (m_depth_stencil_attachment) {
		glDeleteRenderbuffers(1, &m_depth_stencil_attachment.value());
	}
	if (m_fb) {
		glDeleteFramebuffers(1, &m_fb.value());
	}
	if (m_s) {
		m_s.value().stop();
	}

	m_colour_attachment = std::nullopt;
	m_depth_stencil_attachment = std::nullopt;
	m_fb = std::nullopt;
	m_s = std::nullopt;
	m_height = 0;
	m_width = 0;
}
inline void FrameBuffer::bind()
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_fb) {
			std::cerr
				<< "FrameBuffer failed, trying to \"bind\" "
				<< "an unitialised frame buffer.";
			exit(EXIT_FAILURE);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_fb.value());
	glViewport(0, 0, m_width, m_height);
}
inline void FrameBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


class DepthFrameBuffer {
	std::optional<uint32_t> m_fb;
	std::optional<uint32_t> m_depth_attachment;

	uint32_t m_width, m_height;
public:
	void init(uint32_t width, uint32_t height);
	void stop();
	void bind();
	void clearBuffer();

	~DepthFrameBuffer() { stop(); }

	friend class ScreenFrameBuffer;
};

inline void DepthFrameBuffer::clearBuffer()
{
	this->bind();
	glClear(GL_DEPTH_BUFFER_BIT);
}
inline void DepthFrameBuffer::bind()
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_fb) {
			std::cerr
				<< "DepthFrameBuffer failed, trying to \"bind\" "
				<< "an unitialised frame buffer.";
			exit(EXIT_FAILURE);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_fb.value());
	glViewport(0, 0, m_width, m_height);
}
inline void DepthFrameBuffer::init(uint32_t width, uint32_t height)
{
	stop();
	m_width = width;
	m_height = height;

	{ // generate and bind, check for error.
		m_fb = 0;
		glGenFramebuffers(1, &m_fb.value());
		m_fb = (m_fb.value() == 0) ? std::nullopt : m_fb;
		bind();
	}

	{ // geneate texture/colour attachment.
		m_depth_attachment = 0;
		glGenTextures(1, &m_depth_attachment.value());
		glBindTexture(GL_TEXTURE_2D, m_depth_attachment.value());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		// Set texture parameters here (e.g., GL_LINEAR, GL_CLAMP_TO_EDGE)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_depth_attachment.value());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment.value(), 0);
		/*glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);*/
	}

	{ // validate the creation
		if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
			bind();
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Failed to init DepthFrameBuffer";
				m_fb = std::nullopt;
				m_depth_attachment = std::nullopt;
			}
		}
	}
}
inline void DepthFrameBuffer::stop()
{
	if (m_depth_attachment) {
		glDeleteTextures(1, &m_depth_attachment.value());
	}

	if (m_fb) {
		glDeleteFramebuffers(1, &m_depth_attachment.value());
	}

	m_depth_attachment = std::nullopt;
	m_fb = std::nullopt;
	m_height = 0;
	m_width = 0;
}

class ScreenFrameBuffer {
	constexpr static uint32_t m_fb = 0;
	static VertexBuffer m_vb;
	static VertexArray m_va;
	static Shader m_s_basic;
	static Shader m_s_depth;

	constexpr static auto getLayout() -> VertexBufferLayout
	{
		VertexBufferLayout vbl;
		vbl.push<float>(2);
		vbl.push<float>(2);
		return vbl;
	}

public:
	static void bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
	}
	static void init()
	{
		ScreenFrameBuffer::stop();
		m_vb.init();
		m_va.init();
		m_s_basic.init("assets/shaders/screen.vert.glsl", "assets/shaders/screen_basic.frag.glsl", std::nullopt);
		m_s_depth.init("assets/shaders/screen.vert.glsl", "assets/shaders/screen_depth.frag.glsl", std::nullopt);

		m_s_basic.uploadToGpu();
		m_s_depth.uploadToGpu();

		auto layout = getLayout();
		m_va.attachBufferAndLayout(m_vb, layout);
		m_vb.unbind();
		m_va.unbind();
		m_s_basic.unbind();
		m_s_depth.unbind();
	}

	static void stop()
	{
		m_vb.stop();
		m_va.stop();
		m_s_basic.stop();
		m_s_depth.stop();

	}

	static auto shaderBasic() -> Shader&
	{
		return m_s_basic;
	}

	static void draw(FrameBuffer& fb)
	{
		ScreenFrameBuffer::bind();

		m_va.bind();
		m_vb.bind();
		m_s_basic.bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fb.m_colour_attachment.value());

		constexpr static auto vertex_buffer_data = std::to_array({ -1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f });
		m_vb.loadVertices(vertex_buffer_data);

		m_s_basic.setUniform("u_screen_texture", 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		m_va.unbind();
		m_vb.unbind();
		m_s_basic.unbind();
	}

	static void draw(DepthFrameBuffer& fb)
	{
		ScreenFrameBuffer::bind();

		m_va.bind();
		m_vb.bind();
		m_s_depth.bind();

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, fb.m_depth_attachment.value());

		constexpr static auto vertex_buffer_data = std::to_array({ -1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f });

		m_s_depth.setUniform("u_screen_texture", 0);

		m_vb.loadVertices(vertex_buffer_data);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		m_va.unbind();
		m_vb.unbind();
		m_s_depth.unbind();
	}
};

inline std::optional<VertexBuffer> FrameBuffer::m_vb = {};
inline std::optional<VertexArray> FrameBuffer::m_va = {};

inline VertexBuffer ScreenFrameBuffer::m_vb = {};
inline VertexArray ScreenFrameBuffer::m_va = {};
inline Shader ScreenFrameBuffer::m_s_basic = {};
inline Shader ScreenFrameBuffer::m_s_depth = {};

