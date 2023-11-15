#include "Application.hpp"

#include <array>
#include <string_view>

#include "Loader.hpp"
#include "renderer/core/FrameBuffer.hpp"

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

// globals for emscription render loop.
std::chrono::steady_clock::time_point last_time;
OpenglContext* main_context_ptr = nullptr;
uint32_t last_width { 0 }, last_height { 0 };
FrameBuffer* offscreen_fb_ptr = nullptr;
FrameBuffer* post_process_bloom_fb_ptr = nullptr;
FrameBuffer* post_process_other_fb_ptr = nullptr;
DepthFrameBuffer* depth_fb_ptr = nullptr;
DepthFrameBuffer* light_depth_fb_ptr = nullptr;
Scene* scene_ptr = nullptr;
Renderer* renderer_ptr = nullptr;
Input* input_ptr = nullptr;

constexpr auto scenes = std::to_array({ std::string_view { "assets/scenes/basic.json" },
    std::string_view { "assets/scenes/wireframe.json" },
    std::string_view { "assets/scenes/normals.json" },
    std::string_view { "assets/scenes/phong.json" },
    std::string_view { "assets/scenes/gooch.json" },
    std::string_view { "assets/scenes/texture.json" } });

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

    last_time = std::chrono::high_resolution_clock::now();

    FrameBuffer offscreen_fb;
    FrameBuffer post_process_bloom_fb;
    FrameBuffer post_process_other_fb;
    DepthFrameBuffer depth_fb;
    DepthFrameBuffer light_depth_fb;



    main_context_ptr = &m_main_context;

    offscreen_fb_ptr = &offscreen_fb;
    post_process_bloom_fb_ptr = &post_process_bloom_fb;
    post_process_other_fb_ptr = &post_process_other_fb;
    depth_fb_ptr = &depth_fb;
    light_depth_fb_ptr = &light_depth_fb;
	scene_ptr = &m_scene;
	renderer_ptr = &m_renderer;
	input_ptr = &m_input;

    auto render = []() {
    	static bool has_bloom_post_processing = false;
    	static bool has_other_post_processing = false;

        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed_seconds = current_time - last_time;
        float dt = elapsed_seconds.count();
        last_time = current_time;

        // auto [width, height] = main_context_ptr->getScreenDimensions();
        // width = (width == 0) ? 1 : width;
        // height = (height == 0) ? 1 : height;
        float width = 800;
        float height = 800;

        if (last_width != width || last_height != height) {
            last_width = width;
            last_height = height;

            offscreen_fb_ptr->init(width, height);
            post_process_bloom_fb_ptr->init(width, height);
            post_process_other_fb_ptr->init(width, height);
            depth_fb_ptr->init(width, height);
            light_depth_fb_ptr->init(width, height);
        }

        // render standard objects.
        {
            offscreen_fb_ptr->clearBuffer();
            renderer_ptr->draw(*scene_ptr, (float)width, (float)height);
        }

        // render depth buffer
        {
            depth_fb_ptr->clearBuffer();
            renderer_ptr->draw(*scene_ptr, (float)width, (float)height);
        }

        // render light shadows
        {
            light_depth_fb_ptr->clearBuffer();
            renderer_ptr->drawShadows(*scene_ptr, (float)width, (float)height);
        }

        // post process that bloom.
        {
            post_process_bloom_fb_ptr->clearBuffer();
            post_process_bloom_fb_ptr->draw(*offscreen_fb_ptr, "assets/shaders/bloom.frag.glsl");
        }

        // post process then rest.
        {
            post_process_other_fb_ptr->clearBuffer();
            if (has_bloom_post_processing) {
                post_process_other_fb_ptr->draw(*post_process_bloom_fb_ptr, "assets/shaders/post_process_mix.frag.glsl");
            } else {
                post_process_other_fb_ptr->draw(*offscreen_fb_ptr, "assets/shaders/post_process_mix.frag.glsl");
            }
        }

        // overlay bloom ontop offscreen pass.
        {
            ScreenFrameBuffer::bind();
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            if (has_other_post_processing) {
                ScreenFrameBuffer::draw(*post_process_other_fb_ptr);
            } else if (has_bloom_post_processing) {
                ScreenFrameBuffer::draw(*post_process_bloom_fb_ptr);
            } else {
                ScreenFrameBuffer::draw(*offscreen_fb_ptr);
            }
            // ScreenFrameBuffer::draw(depth_fb);

            main_context_ptr->swapBuffers();
        }

        scene_ptr->update(dt, *input_ptr);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - current_time).count();

        if (input_ptr->isKeyDown('1')) {
            std::cout << "1\n";
            scene_ptr->init(scenes[0]);
        } else if (input_ptr->isKeyDown('2')) {
            std::cout << "2\n";
            scene_ptr->init(scenes[1]);
        } else if (input_ptr->isKeyDown('3')) {
            std::cout << "3\n";
            scene_ptr->init(scenes[2]);
        } else if (input_ptr->isKeyDown('4')) {
            std::cout << "4\n";
            scene_ptr->init(scenes[3]);
        } else if (input_ptr->isKeyDown('5')) {
            std::cout << "5\n";
            scene_ptr->init(scenes[4]);
        } else if (input_ptr->isKeyDown('6')) {
            std::cout << "6\n";
            scene_ptr->init(scenes[5]);
        } else if (input_ptr->isKeyDown('7')) {
            SerialisedScene sscene;
            if (auto has_result = sscene.load("assets/scenes/entity_scene.json"); has_result.HasError()) {
                sscene.loadIntoScene(scene_ptr);
            } else {
                std::cerr 
                    << has_result.Error() 
                    << '\n' 
                    << std::endl;
                assert(false);
                exit(EXIT_FAILURE);
            }
        } else if (input_ptr->isKeyDown('R')) {
            auto start_time_reload = std::chrono::high_resolution_clock::now();
            scene_ptr->reload();
            auto end_time_reload = std::chrono::high_resolution_clock::now();
            auto elapsed_reload = std::chrono::duration_cast<std::chrono::microseconds>(end_time - current_time).count();
            std::cout << "Reloaded. Took " << elapsed_reload / 1000.0f << "ms\n";
            ScreenFrameBuffer::shaderBasic().reload();
        } else if (input_ptr->isKeyDown('B')) {
            static auto b_timer = std::chrono::high_resolution_clock::now();
            auto current_time = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - b_timer).count() > 200) {
                has_bloom_post_processing = !(has_bloom_post_processing);
                b_timer = current_time;
            }
        } else if (input_ptr->isKeyDown('P')) {
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
    };

#if BUILD_TARGET == NATIVE_BUILD
    while (!m_main_context.shouldClose() && !m_input.isKeyDown(Input::Key::escape)) {
        render();
        m_main_context.pollEvents();
    }
#elif BUILD_TARGET == WEB_BUILD
    void (*renderFunc)() = render;
    emscripten_set_main_loop(renderFunc, 0, 1);
#endif

    return {};
}

void Application::stop() noexcept
{
    ScreenFrameBuffer::stop();
    m_main_context.stop();
    glfwTerminate();
}
