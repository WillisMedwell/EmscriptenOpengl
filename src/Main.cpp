#include <array>
#include <glm/glm.hpp>
#include <iostream>


#include "Application.hpp"

#include "Loader.hpp"
#include "renderer/3d/Scene.hpp"
#include <glaze/glaze.hpp>


auto printAndQuit = [](auto msg) -> Expected<void, std::string_view> {
    std::cerr << msg;
    exit(EXIT_FAILURE);
    return {};
};

#include "Ecs.hpp"
int main()
{
    Application app;

    app.init().OnError(printAndQuit).OnValue([]() { std::cout << "Done init.\n"; });
    app.run().OnError(printAndQuit).OnValue([]() { std::cout << "Done running.\n"; });
    app.stop();

    return 0;
}