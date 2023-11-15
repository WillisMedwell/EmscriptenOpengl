#include "Application.hpp"

#include <array>
#include <string_view>

#include "renderer/core/FrameBuffer.hpp"
#include "Loader.hpp"

static inline size_t allocations = 0;
static inline size_t size = 0;

void* operator new(size_t n)
{
	allocations++;
	size += n;
	return malloc(n);
}

#include "renderer/3d/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


constexpr auto scenes = std::to_array({ std::string_view { "assets/scenes/basic.json" },
	std::string_view { "assets/scenes/wireframe.json" },
	std::string_view { "assets/scenes/normals.json" },
	std::string_view { "assets/scenes/phong.json" },
	std::string_view { "assets/scenes/gooch.json" },
	std::string_view { "assets/scenes/texture.json" } }
);

auto Application::init() noexcept -> Expected<void, std::string_view>
{
	// Init GLFW
	if (!glfwInit()) {
		static constexpr std::string_view msg = "Unable to initialise glfw library.";
		return { msg };
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create context/window
	if (auto init_context = m_main_context.init("App", 800, 800); init_context.HasError()) {
		return { init_context.Error() };
	}

	if (auto init_scene = m_scene.init("assets/scenes/normals.json"); init_scene.HasError()) {
		return { init_scene.Error() };
	}


	m_renderer.init();
	m_input.init(&m_main_context);
	ScreenFrameBuffer::init();

	return {};
}

auto Application::run() noexcept -> Expected<void, std::string_view>
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_main_context.isInit()) {
			static constexpr auto msg = "Unable to run, main context has not been initialized";
			return { msg };
		}
	}

	auto last_time = std::chrono::high_resolution_clock::now();
	uint32_t last_width{ 0 }, last_height{ 0 };
	FrameBuffer offscreen_fb;
	FrameBuffer post_process_bloom_fb;
	FrameBuffer post_process_other_fb;
	DepthFrameBuffer depth_fb;
	DepthFrameBuffer light_depth_fb;

	bool has_bloom_post_processing = false;
	bool has_other_post_processing = false;

	while (!m_main_context.shouldClose() && !m_input.isKeyDown(Input::Key::escape)) {
		auto current_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed_seconds = current_time - last_time;
		float dt = elapsed_seconds.count();
		last_time = current_time;

		m_main_context.pollEvents();
		auto [width, height] = m_main_context.getScreenDimensions();
		width = (width == 0) ? 1 : width;
		height = (height == 0) ? 1 : height;

		if (last_width != width || last_height != height) {
			last_width = width;
			last_height = height;

			offscreen_fb.init(width, height);
			post_process_bloom_fb.init(width, height);
			post_process_other_fb.init(width, height);
			depth_fb.init(width, height);
			light_depth_fb.init(width, height);
		}

		// render standard objects.
		{
			offscreen_fb.clearBuffer();
			m_renderer.draw(m_scene, (float)width, (float)height);
		}

		// render depth buffer
		{
			depth_fb.clearBuffer();
			m_renderer.draw(m_scene, (float)width, (float)height);
		}

		// render light shadows
		{
			light_depth_fb.clearBuffer();
			m_renderer.drawShadows(m_scene, (float)width, (float)height);
		}

		// post process that bloom.
		{
			post_process_bloom_fb.clearBuffer();
			post_process_bloom_fb.draw(offscreen_fb, "assets/shaders/bloom.frag.glsl");
		}

		// post process then rest.
		{
			post_process_other_fb.clearBuffer();
			if (has_bloom_post_processing) {
				post_process_other_fb.draw(post_process_bloom_fb, "assets/shaders/post_process_mix.frag.glsl");
			}
			else {
				post_process_other_fb.draw(offscreen_fb, "assets/shaders/post_process_mix.frag.glsl");
			}
		}

		// overlay bloom ontop offscreen pass.
		{
			ScreenFrameBuffer::bind();
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			if (has_other_post_processing) {
				ScreenFrameBuffer::draw(post_process_other_fb);
			}
			else if (has_bloom_post_processing) {
				ScreenFrameBuffer::draw(post_process_bloom_fb);
			}
			else {
				ScreenFrameBuffer::draw(offscreen_fb);
			}
			//ScreenFrameBuffer::draw(depth_fb);

			m_main_context.swapBuffers();
		}

		m_scene.update(dt, m_input);

		auto end_time = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - current_time).count();

		if (m_input.isKeyDown('1')) {
			std::cout << "1\n";
			m_scene.init(scenes[0]);
		}
		else if (m_input.isKeyDown('2')) {
			std::cout << "2\n";
			m_scene.init(scenes[1]);
		}
		else if (m_input.isKeyDown('3')) {
			std::cout << "3\n";
			m_scene.init(scenes[2]);
		}
		else if (m_input.isKeyDown('4')) {
			std::cout << "4\n";
			m_scene.init(scenes[3]);
		}
		else if (m_input.isKeyDown('5')) {
			std::cout << "5\n";
			m_scene.init(scenes[4]);
		}
		else if (m_input.isKeyDown('6')) {
			std::cout << "6\n";
			m_scene.init(scenes[5]);
		}
		else if (m_input.isKeyDown('7')) {
			SerialisedScene sscene;
			if(auto has_result = sscene.load("assets/scenes/entity_scene.json"); has_result.HasError()) {
				sscene.loadIntoScene(&m_scene);
			}
			else {
				std::cerr << has_result.Error();
				assert(false);
				exit(EXIT_FAILURE);
			}
		}
		else if (m_input.isKeyDown('R')) {
			auto start_time_reload = std::chrono::high_resolution_clock::now();
			m_scene.reload();
			auto end_time_reload = std::chrono::high_resolution_clock::now();
			auto elapsed_reload = std::chrono::duration_cast<std::chrono::microseconds>(end_time - current_time).count();
			std::cout << "Reloaded. Took " << elapsed_reload / 1000.0f << "ms\n";
			ScreenFrameBuffer::shaderBasic().reload();
		}
		else if (m_input.isKeyDown('B')) {
			static auto b_timer = std::chrono::high_resolution_clock::now();
			auto current_time = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - b_timer).count() > 200) {
				has_bloom_post_processing = !(has_bloom_post_processing);
				b_timer = current_time;
			}
		}
		else if (m_input.isKeyDown('P')) {
			static auto p_timer = std::chrono::high_resolution_clock::now();
			auto current_time = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - p_timer).count() > 200) {
				has_other_post_processing = !(has_other_post_processing);
				p_timer = current_time;
			}
		}

		allocations = 0;
		size = 0;

		auto elapsed_reload = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - current_time).count();
		//std::cout << "Frame Time " << elapsed_reload / 1000.0f << "ms\n";

		//std::cout << "pos: " <<m_scene.camera.camera_pos.x << "," << m_scene.camera.camera_pos.y << "," << m_scene.camera.camera_pos.z << "\n";
		//std::cout << "dir: " << m_scene.camera.camera_dir.x << "," << m_scene.camera.camera_dir.y << "," << m_scene.camera.camera_dir.z << "\n";
	}

	return {};
}

void Application::stop() noexcept
{
	ScreenFrameBuffer::stop();
	m_main_context.stop();
	glfwTerminate();
}
