#include <cstddef>
#include <exception>
#include <iostream>

#include "lake.hpp"

static std::size_t width  = 100;
static std::size_t height = 60;

using namespace lake_logic;

int main() {
	try {
		Lake lake(width, height);

		Path empty_path; // no path constructed yet; corners are Type::EMPTY, so no relocation needed

		Path path = getComplexPath(lake, 45.0, 2.0);

		lake.generateHoles(0.5, path); // holes are excluded from cells already on the path

		lake.print();
		lake.print(path);
	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	return 0;
}