#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>
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
				
		[[nodiscard]] bool isValid() const noexcept {
			if (path_.empty() || path_.front() == nullptr) { return false; }
			for (std::size_t i = 1; i < path_.size(); ++i) {
				if (path_[i] == nullptr) { return false; }
				if (path_[i - 1]->isAdjacent(path_[i]) == Direction::ERROR) { return false; }
			}
			return true;
		}

		[[nodiscard]] bool isComplete() const noexcept {
			if (!isValid()) { return false; }
			return path_.front()->getType() == Type::START
				&& path_.back()->getType() == Type::GOAL;
		}

		void print() const noexcept {
			std::cout << "Path: length: " << getColorStr(Color::RED) << size() << getColorStr(Color::RESET)
				<< ", start: " << getTypeStr(front()->getType()) << getColorStr(Color::RESET)
				<< ", end: " << getTypeStr(back()->getType()) << getColorStr(Color::RESET)
				<< ", isValid: " << (isValid() ? getColorStr(Color::GREEN) + "Yes" + getColorStr(Color::RESET) :
					getColorStr(Color::RED) + "No" + getColorStr(Color::RESET))
				<< ", isComplete: " << (isComplete() ? getColorStr(Color::GREEN) + "Yes" + getColorStr(Color::RESET)
 					: getColorStr(Color::RED) + "No" + getColorStr(Color::RESET)) << "\n";

			std::size_t n = 0;
			for (auto& cell : path_) { std::cout << ++n << ". "; cell->print(); std::cout << "\n"; }
		}
	};

	// Declared here, defaults included once; defined below, after Lake is complete.
	[[nodiscard]] Path getComplexPath(const Lake& lake, double amplitude = 5.0, double cycles = 3.0);

} // namespace lake_logic

#include "lake.hpp" // Lake needed for getComplexPath's body below

namespace lake_logic {

	// Walks a 4-connected (no diagonal) grid path from (from_x, from_y) toward
	// (to_x, to_y), appending every intermediate cell to path. At each step,
	// moves along whichever axis currently has the larger remaining distance —
	// this lets the path both increase and decrease in either coordinate,
	// which is what a true sine-wave deviation (not just a monotonic
	// approach) requires.
	inline void walkSegment(
		const Lake& lake,
		Path& path,
		long long from_x, long long from_y,
		const long long to_x, const long long to_y) {
		while (from_x != to_x || from_y != to_y) {
			const long long dx = to_x - from_x;
			const long long dy = to_y - from_y;
			if (std::abs(dx) >= std::abs(dy) && dx != 0) {
				from_x += (dx > 0) ? 1 : -1;
			} else if (dy != 0) {
				from_y += (dy > 0) ? 1 : -1;
			} else {
				from_x += (dx > 0) ? 1 : -1;
			}
			// const_cast: Lake is taken as const& here, but Path stores mutable
			// Cell* so callers (e.g. MC updates) can write Q-values through it
			// regardless of the Lake reference's own constness.
			path.addCell(const_cast<Cell*>(
				&lake.getCell(static_cast<std::size_t>(from_x), static_cast<std::size_t>(from_y))));
		}
	}

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
		const Comp dir = goal - start;
		const double len = std::abs(dir);

		path.addCell(start_cell);
		if (len < 1e-9) {
			return path;
		}

		const Comp dir_norm = dir / len;
		// Perpendicular unit vector: rotate dir_norm by +90 degrees.
		const Comp perp(-dir_norm.imag(), dir_norm.real());

		constexpr double pi = 3.14159265358979323846;
		const double omega = (2.0 * pi * cycles) / len;

		const long long max_x = static_cast<long long>(lake.getWidth()) - 1;
		const long long max_y = static_cast<long long>(lake.getHeight()) - 1;

		// Sample the parametric curve start + t*dir_norm + amplitude*sin(omega*t)*perp
		// roughly once per unit of travel along the direction axis, then connect
		// consecutive samples with a 4-connected segment. This is what allows the
		// path to actually curve away from and back toward the straight line,
		// rather than only ever approaching the goal monotonically.
		const std::size_t num_samples = static_cast<std::size_t>(std::ceil(len)) + 1;

		long long curr_x = static_cast<long long>(start_x);
		long long curr_y = static_cast<long long>(start_y);

		for (std::size_t i = 1; i < num_samples; ++i) {
			const double t = std::min(static_cast<double>(i), len);
			const double offset = amplitude * std::sin(omega * t);
			const Comp p = start + dir_norm * t + perp * offset;

			const bool is_last = (i == num_samples - 1);
			const long long target_x = is_last
				? static_cast<long long>(goal_x)
				: std::clamp(std::llround(p.real()), 0LL, max_x);
			const long long target_y = is_last
				? static_cast<long long>(goal_y)
				: std::clamp(std::llround(p.imag()), 0LL, max_y);

			walkSegment(lake, path, curr_x, curr_y, target_x, target_y);
			curr_x = target_x;
			curr_y = target_y;
		}

		return path;
	}

} // namespace lake_logic