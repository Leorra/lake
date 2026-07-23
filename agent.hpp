#pragma once

#include <cstddef>
#include <random>
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
		Cell* goal_cell_ { nullptr };

		std::mt19937 rng_;

		// greeks
		double epsilon_ { 0.1 };
		double gamma_ { 0.9999 };
		double step_reward_ { -0.01 };

	public:
		Agent(Lake& lake) : lake_(lake),
			start_cell_(lake.getStart()),
			goal_cell_(lake.getGoal()),
			rng_(std::random_device {}()) {
			if (start_cell_ == nullptr || goal_cell_ == nullptr) {
				throw std::invalid_argument("Agent(): Start or goal cell is nullptr");
			}
			current_cell_ = start_cell_;
		}

		[[nodiscard]] Path getEpisode(std::size_t max_num) {
			current_cell_ = start_cell_;
			Path path; path.addCell(current_cell_);
			for (std::size_t n = 0; n < max_num; ++n) {
				if (current_cell_->isTerminal()) { break; }
				auto dir_opt = current_cell_->getBestDirection(epsilon_, rng_);
				if (!dir_opt.has_value()) { break; }
				current_cell_ = current_cell_->getNext(dir_opt.value());
				path.addCell(current_cell_);
			}

			return path;
		}
	};

} // namespace lake_logic