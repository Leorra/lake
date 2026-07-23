#include <cstddef>
#include <exception>
#include <iostream>

#include "lake.hpp"
#include "path.hpp"

static std::size_t width = 20;
static std::size_t height = 20;

using namespace lake_logic;

int main() {
	try {
		Lake lake(width, height);

		Path path; // no path constructed yet; corners are Type::EMPTY, so no relocation needed
		lake.setStartCell(0, 0, path);
		lake.setGoalCell(width - 1, height - 1, path);

		path = getComplexPath(lake, 2.0, 1.0);

		lake.generateHoles(0.25, path); // holes are excluded from cells already on the path

		lake.print();
		lake.print(path);
	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	return 0;
}