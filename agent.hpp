#pragma once

#include <cstddef>
#include <stdexcept>

#include "cell.hpp"
#include "lake.hpp"
#include "path.hpp"

namespace lake_logic {

	class Agent {
	private:
		std::size_t x_ { 0 };
		std::size_t y_ { 0 };

		Lake& lake_;

		Cell* start_cell_ { nullptr };
		Cell* goal_cell_  { nullptr };

	public:
		Agent(Lake& lake) : lake_(lake),
			start_cell_(lake.getStart()),
			goal_cell_(lake.getGoal()) {
			if (start_cell_ == nullptr || goal_cell_ == nullptr) {
				throw std::invalid_argument("Agent(): Start or goal cell is nullptr");
			}
		}
	};

} // namespace lake_logic