#pragma once

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "cell.hpp"

namespace lake_logic {

	class Path; // forward declaration; complete type only needed in the method bodies below

	class Lake {
	private:
		std::size_t width_ { 0 };
		std::size_t height_ { 0 };
		std::vector<Cell> cells_;
		double hole_probability_ { 0.0 };
		std::size_t total_holes_ { 0 };
		std::mt19937 rng_;
		Cell* start_ { nullptr };
		Cell* goal_ { nullptr };
		std::vector<Cell*> candidates_buffer_;

		Cell* getRandomEmptyCell(const Path& path); // defined below, after Path is complete

	public:
		Lake(std::size_t width, std::size_t height);

		Lake(const Lake&) = delete;
		Lake& operator=(const Lake&) = delete;
		Lake(Lake&&) = delete;
		Lake& operator=(Lake&&) = delete;

		[[nodiscard]] Cell& getCell(std::size_t x, std::size_t y);
		[[nodiscard]] const Cell& getCell(std::size_t x, std::size_t y) const;

		[[nodiscard]] std::size_t getWidth() const noexcept { return width_; }
		[[nodiscard]] std::size_t getHeight() const noexcept { return height_; }
		[[nodiscard]] std::size_t size() const noexcept { return cells_.size(); }

		[[nodiscard]] Cell* getStart() const noexcept { return start_; }
		[[nodiscard]] Cell* getGoal() const noexcept { return goal_; }

		// path is the already-existing route (may be a default-constructed, empty
		// Path) whose cells must be excluded when relocating a displaced occupant.
		void setStartCell(std::size_t x, std::size_t y, const Path& path);
		void setGoalCell(std::size_t x, std::size_t y, const Path& path);
		void generateHoles(double hole_probability, const Path& path);

		void print() const;
		void print(const Path& path) const; // defined below, after Path is complete
	};

	inline Lake::Lake(const std::size_t width, const std::size_t height) :
		width_(width), height_(height),
		rng_(std::random_device {}()) {
		cells_.reserve(width_ * height_);
		candidates_buffer_.reserve(width_ * height_);

		for (std::size_t y = 0; y < height_; ++y) {
			for (std::size_t x = 0; x < width_; ++x) {
				cells_.emplace_back(x, y);
				auto& cell = cells_.back();

				if (x > 0) {
					auto& left = getCell(x - 1, y);
					cell.setNext(Direction::LEFT, &left);
					left.setNext(Direction::RIGHT, &cell);
				}
				if (y > 0) {
					auto& up = getCell(x, y - 1);
					cell.setNext(Direction::UP, &up);
					up.setNext(Direction::DOWN, &cell);
				}
			}
		}
	}

	inline Cell& Lake::getCell(const std::size_t x, const std::size_t y) {
		if (x >= width_ || y >= height_) {
			throw std::out_of_range("[EXCEPTION: getCell()] Cell index out of bounds: x="
				+ std::to_string(x) + ", y=" + std::to_string(y));
		}
		return cells_[y * width_ + x];
	}

	inline const Cell& Lake::getCell(const std::size_t x, const std::size_t y) const {
		if (x >= width_ || y >= height_) {
			throw std::out_of_range("[EXCEPTION: getCell()] Cell index out of bounds: x="
				+ std::to_string(x) + ", y=" + std::to_string(y));
		}
		return cells_[y * width_ + x];
	}

	inline void Lake::print() const {
		for (std::size_t y = 0; y < height_; ++y) {
			std::string row;
			row.reserve(width_ * 8);
			for (std::size_t x = 0; x < width_; ++x) {
				row.append(getTypeStr(getCell(x, y).getType()));
			}
			std::cout << row << getColorStr(Color::RESET) << "\n";
		}
	}

} // namespace lake_logic

#include "path.hpp" // Path needed for the bodies below

namespace lake_logic {

	inline Cell* Lake::getRandomEmptyCell(const Path& path) {
		candidates_buffer_.clear();
		for (auto& cell : cells_) {
			if (cell.getType() == Type::EMPTY && !path.isOnPath(&cell)) {
				candidates_buffer_.push_back(&cell);
			}
		}
		if (candidates_buffer_.empty()) { return nullptr; }
		std::uniform_int_distribution<std::size_t> dist(0, candidates_buffer_.size() - 1);
		return candidates_buffer_[dist(rng_)];
	}

	inline void Lake::setStartCell(const std::size_t x, const std::size_t y, const Path& path) {
		Cell& cell = getCell(x, y);
		if (cell.getType() != Type::EMPTY) {
			Cell* const empty_cell = getRandomEmptyCell(path);
			if (empty_cell == nullptr) {
				throw std::logic_error("setStartCell(): no empty cells found");
			}
			empty_cell->setType(cell.getType());
		}
		cell.setType(Type::START);
		start_ = &cell;
	}

	inline void Lake::setGoalCell(const std::size_t x, const std::size_t y, const Path& path) {
		Cell& cell = getCell(x, y);
		if (cell.getType() != Type::EMPTY) {
			Cell* const empty_cell = getRandomEmptyCell(path);
			if (empty_cell == nullptr) {
				throw std::logic_error("setGoalCell(): no empty cells found");
			}
			empty_cell->setType(cell.getType());
		}
		cell.setType(Type::GOAL);
		goal_ = &cell;
	}

	inline void Lake::generateHoles(const double hole_probability, const Path& path) {
		hole_probability_ = std::clamp(hole_probability, 0.0, 1.0);
		const std::size_t total_cells = width_ * height_;
		if (total_cells <= 2) { return; }

		total_holes_ = static_cast<std::size_t>(static_cast<double>(total_cells) * hole_probability_);
		total_holes_ = std::min(total_holes_, total_cells - 2);

		std::size_t holes_count = 0;
		while (holes_count < total_holes_) {
			Cell* cell = getRandomEmptyCell(path);
			if (cell == nullptr) { break; }
			cell->setType(Type::HOLE);
			++holes_count;
		}
		total_holes_ = holes_count;
	}

	inline void Lake::print(const Path& path) const {
		std::cout << "Lake: " << getColorStr(Color::GREEN) << "[" << width_ << "x" << height_ << "]"
			<< getColorStr(Color::RESET) << ", Holes: " << getColorStr(Color::BLUE) << total_holes_
			<< getColorStr(Color::RESET) << " (" << std::fixed << std::setprecision(2)
			<< getColorStr(Color::BLUE) << hole_probability_ << getColorStr(Color::RESET) << ")\n";

		std::size_t holes_count = 0;
		for (std::size_t y = 0; y < height_; ++y) {
			std::string row;
			row.reserve(width_ * 8);
			for (std::size_t x = 0; x < width_; ++x) {
				const Cell& cell = getCell(x, y);
				if (cell.getType() != Type::START	&& cell.getType() != Type::GOAL && path.isOnPath(&cell)) {
					row.append(getTypeStr(Type::PATH));
					continue;
				}
				if (cell.getType() == Type::HOLE) { ++holes_count; }
				row.append(getTypeStr(cell.getType()));
			}
			std::cout << row << getColorStr(Color::RESET) << "\n";
		}

		std::cout << "Holes count: "
			<< ((holes_count == total_holes_) ? getColorStr(Color::GREEN) : getColorStr(Color::RED))
			<< holes_count << getColorStr(Color::RESET) << "\n";
	}

} // namespace lake_logic