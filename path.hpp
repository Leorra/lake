#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "cell.hpp"

namespace lake_logic {

	class Lake; // forward declaration; complete type only needed in getComplexPath's body, below

	class Path {
	private:
		std::vector<Cell*> path_ {};

	public:
		Path() = default;

		void addCell(Cell* cell) { path_.push_back(cell); }
		void clear() noexcept { path_.clear(); }

		[[nodiscard]] bool empty() const noexcept { return path_.empty(); }
		[[nodiscard]] std::size_t size() const noexcept { return path_.size(); }

		[[nodiscard]] Cell* operator[](std::size_t index) const noexcept { return path_[index]; }
		[[nodiscard]] Cell* front() const noexcept { return path_.front(); }
		[[nodiscard]] Cell* back() const noexcept { return path_.back(); }

		[[nodiscard]] auto begin() const noexcept { return path_.begin(); }
		[[nodiscard]] auto end() const noexcept { return path_.end(); }

		[[nodiscard]] bool isOnPath(const Cell* cell) const noexcept {
			if (cell == nullptr) { return false; }
			return std::find(path_.begin(), path_.end(), cell) != path_.end();
		}

		[[nodiscard]] static bool isAdjacent(const Cell* from, const Cell* to) noexcept {
			if (from == nullptr || to == nullptr) { return false; }
			for (std::size_t i = 0; i < kDirectionCount; ++i) {
				if (from->getNext(static_cast<Direction>(i)) == to) { return true; }
			}
			return false;
		}

		[[nodiscard]] bool isValid() const noexcept {
			if (path_.empty() || path_.front() == nullptr) { return false; }
			for (std::size_t i = 1; i < path_.size(); ++i) {
				if (path_[i] == nullptr) { return false; }
				if (!isAdjacent(path_[i - 1], path_[i])) { return false; }
			}
			return true;
		}

		[[nodiscard]] bool isCompletePath() const noexcept {
			if (!isValid()) { return false; }
			return path_.front()->getType() == Type::START
				&& path_.back()->getType() == Type::GOAL;
		}
	};

	// Declared here, defaults included once; defined below, after Lake is complete.
	[[nodiscard]] Path getComplexPath(const Lake& lake, double amplitude = 5.0, double cycles = 3.0);

} // namespace lake_logic

#include "lake.hpp" // Lake needed for getComplexPath's body below

namespace lake_logic {

	inline Path getComplexPath(const Lake& lake, const double amplitude, const double cycles) {
		Path path;

		Cell* start_cell = lake.getStart();
		Cell* goal_cell = lake.getGoal();
		if (!start_cell || !goal_cell) {
			throw std::runtime_error("getComplexPath(): Start or Goal cell is not set");
		}

		const std::size_t start_x = start_cell->getX();
		const std::size_t start_y = start_cell->getY();
		const std::size_t goal_x = goal_cell->getX();
		const std::size_t goal_y = goal_cell->getY();

		using Comp = std::complex<double>;
		const Comp start(static_cast<double>(start_x), static_cast<double>(start_y));
		const Comp goal(static_cast<double>(goal_x), static_cast<double>(goal_y));
		Comp dir = goal - start;
		double len = std::abs(dir);

		if (len < 1e-9) {
			path.addCell(start_cell);
			return path;
		}

		const Comp dir_norm = dir / len;
		const Comp dir_conj = std::conj(dir_norm);
		std::size_t curr_x = start_x;
		std::size_t curr_y = start_y;
		path.addCell(start_cell);

		const int step_x = (goal_x > start_x) ? 1 : ((goal_x < start_x) ? -1 : 0);
		const int step_y = (goal_y > start_y) ? 1 : ((goal_y < start_y) ? -1 : 0);

		constexpr double pi = 3.14159265358979323846;
		const double omega = (2.0 * pi * cycles) / len;

		auto compute_wave_error = [&](const double cell_x, const double cell_y) noexcept {
			Comp delta(cell_x - start.real(), cell_y - start.imag());
			Comp local = delta * dir_conj;
			double t = local.real();
			double n = local.imag();
			double target_offset = amplitude * std::sin(omega * t);
			return std::abs(n - target_offset);
		};

		while (curr_x != goal_x || curr_y != goal_y) {
			double err_x = std::numeric_limits<double>::max();
			double err_y = std::numeric_limits<double>::max();

			if (curr_x != goal_x) {
				err_x = compute_wave_error(
					static_cast<double>(curr_x + step_x),
					static_cast<double>(curr_y)
				);
			}
			if (curr_y != goal_y) {
				err_y = compute_wave_error(
					static_cast<double>(curr_x),
					static_cast<double>(curr_y + step_y)
				);
			}

			if (err_x < err_y) { curr_x += step_x; } else { curr_y += step_y; }

			// const_cast: Lake is taken as const& here, but Path stores mutable
			// Cell* so callers (e.g. MC updates) can write Q-values through it
			// regardless of the Lake reference's own constness.
			path.addCell(const_cast<Cell*>(&lake.getCell(curr_x, curr_y)));
		}

		return path;
	}

} // namespace lake_logic