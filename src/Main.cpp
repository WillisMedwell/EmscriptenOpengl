#include <iostream>
#include <glm/glm.hpp>
#include <array>

#include "Application.hpp"

#include "renderer/3d/Scene.hpp"
#include <glaze/glaze.hpp>
#include "Loader.hpp"

auto printAndQuit = [](auto msg) -> Expected<void, std::string_view>
	{
		std::cerr << msg;
		exit(EXIT_FAILURE);
		return {};
	};

#include "Ecs.hpp"
int main()
{
	SerialisedScene ss;

	ss.entities.emplace_back();
	ss.entities.emplace_back();

	std::cout << glz::write_json(ss) << std::endl;


	Application app;

	app.init().OnError(printAndQuit);
	app.run().OnError(printAndQuit);
	app.stop();

	return 0;
}