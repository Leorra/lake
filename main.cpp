#include <cstddef>
#include <exception>
#include <iostream>

#include "agent.hpp"
#include "lake.hpp"
#include "path.hpp"

static std::size_t width = 80;
static std::size_t height = 40;

using namespace lake_logic;

int main() {
	try {
		Lake lake(width, height);

		Path path = getComplexPath(lake, 5.0, -1.0);

		lake.generateHoles(0.25, path); // holes are excluded from cells already on the path

//		lake.print();
		lake.print(path);
	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	return 0;
}