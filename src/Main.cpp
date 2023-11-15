#include <array>
#include <filesystem>
#include <glm/glm.hpp>
#include <iostream>

#include "Application.hpp"
#include "Ecs.hpp"
#include "Loader.hpp"
#include "renderer/3d/Scene.hpp"


#include <glaze/glaze.hpp>

namespace fs = std::filesystem;

void print_directory(const fs::path& path)
{
    if (fs::exists(path) && fs::is_directory(path)) {
        try {
            for (const auto entry : fs::recursive_directory_iterator(path)) {
                std::cout << entry.path() << std::endl;
            }
        } catch (std::exception& e) {
            
        }
    } else {
        std::cerr << "Provided path is not a directory." << std::endl;
    }
}

auto printAndQuit = [](auto msg) -> Expected<void, std::string_view> {
    std::cerr << msg;
    exit(EXIT_FAILURE);
    return {};
};

int main()
{

    fs::path current_path = fs::current_path();
    print_directory(current_path);

    Application app;

    app.init().OnError(printAndQuit).OnValue([]() { std::cout << "Done init.\n"; });
    app.run().OnError(printAndQuit).OnValue([]() { std::cout << "Done running.\n"; });
    app.stop();

    return 0;
}