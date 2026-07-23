#pragma once

#include <cstddef>
#include <stdexcept>

#include "cell.hpp"
#include "lake.hpp"
#include "path.hpp"

namespace lake_logic {

	class Agent {
	private:

		Lake& lake_;
		Cell* current_cell_ { nullptr };

		Cell* start_cell_ { nullptr };
		Cell* goal_cell_  { nullptr };

		// greecks
		double epsilon_ { 0.1 };
		double gamma_ { 0.9999 };
		double step_reward_ { -0.01 };

	public:
		Agent(Lake& lake) : lake_(lake),
			start_cell_(lake.getStart()),
			goal_cell_(lake.getGoal()) {
			if (start_cell_ == nullptr || goal_cell_ == nullptr) {
				throw std::invalid_argument("Agent(): Start or goal cell is nullptr");
			}
			current_cell_ = start_cell_;
		}
	};

} // namespace lake_logic