#include <cstddef>
#include <exception>
#include <iostream>

#include "agent.hpp"
#include "lake.hpp"
#include "path.hpp"

static std::size_t width  = 5;
static std::size_t height = 5;

using namespace lake_logic;

int main() {
	try {
		Lake lake(width, height);

		Path complex_path = getComplexPath(lake, 2.0, 2.0);
		lake.generateHoles(0.1, complex_path); // holes are excluded from cells already on the path

		Agent agent(lake);
		Path path = agent.getEpisode(1024);

		lake.print(path);
		path.print();

	} catch (const std::exception& e) { 
		std::cerr << e.what() << "\n";
	}

	return 0;
}