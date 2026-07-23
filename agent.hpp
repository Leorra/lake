#pragma once

#include <cstddef>

#include "lake.hpp"
#include "path.hpp"

namespace lake_logic {

	class Agent {
	private:
		std::size_t x_ { 0 };
		std::size_t y_ { 0 };

		Lake& lake_;

	public:
		Agent(Lake& lake) : lake_(lake) {}

	};

} // namespace lake_logic