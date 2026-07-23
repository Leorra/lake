#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <string>

namespace lake_logic {

	enum class Direction : std::size_t { LEFT, UP, RIGHT, DOWN, COUNT };
	enum class Type : std::size_t { EMPTY, HOLE, START, GOAL, PATH };

	enum class Color : std::size_t { WHITE, BLUE, GREEN, RED, YELLOW, RESET };

	inline constexpr double kMinusInfinity = -std::numeric_limits<double>::infinity();
	inline constexpr std::size_t kDirectionCount = static_cast<std::size_t>(Direction::COUNT);

	[[nodiscard]] constexpr double getReward(Type type) noexcept {
		switch (type) {
			case Type::HOLE: return -5.0;
			case Type::GOAL: return 100.0;
			case Type::EMPTY:
			case Type::START:
			case Type::PATH: return 0.0;
		}
		return 0.0; // unreachable given exhaustive switch above
	}

	[[nodiscard]] inline std::string getColorStr(Color color) noexcept {
		switch (color) {
			case Color::WHITE:  return "\033[37m";
			case Color::BLUE:   return "\033[34m";
			case Color::GREEN:  return "\033[32m";
			case Color::RED:    return "\033[31m";
			case Color::YELLOW: return "\033[33m";
			case Color::RESET:  return "\033[0m";
		}
		return "\033[0m";
	}

	[[nodiscard]] inline std::string getTypeStr(Type type) noexcept {
		switch (type) {
			case Type::EMPTY: return getColorStr(Color::WHITE) + ".";
			case Type::HOLE:  return getColorStr(Color::BLUE) + "H";
			case Type::START: return getColorStr(Color::GREEN) + "S";
			case Type::GOAL:  return getColorStr(Color::YELLOW) + "G";
			case Type::PATH:  return getColorStr(Color::RED) + "*";
		}
		return getColorStr(Color::RESET) + "  ";
	}

	class Cell;  // Forward declaration

	struct Link {
		double q_ { 0.0 };
		Cell* next_ { nullptr };
	};

	class Cell {
	private:
		std::size_t x_ { 0 };
		std::size_t y_ { 0 };
		std::array<Link, kDirectionCount> links_ {};
		std::size_t visits_ { 0 };
		Type type_ { Type::EMPTY };

		// Lake is responsible for ensuring pointer stability
		friend class Lake;

		void setNext(Direction dir, Cell* next) noexcept {
			links_[static_cast<std::size_t>(dir)].next_ = next;
		}

	public:
		Cell(const std::size_t x, const std::size_t y) : x_(x), y_(y) {}

		[[nodiscard]] std::size_t getX() const noexcept { return x_; }
		[[nodiscard]] std::size_t getY() const noexcept { return y_; }

		[[nodiscard]] Type getType() const noexcept { return type_; }
		[[nodiscard]] bool isTerminal() const noexcept { return type_ == Type::HOLE || type_ == Type::GOAL; }
		void setType(Type type) noexcept { type_ = type; }
		[[nodiscard]] std::size_t getVisits() const noexcept { return visits_; }

		[[nodiscard]] double getQ(Direction dir) const noexcept {
			const auto& link = links_[static_cast<std::size_t>(dir)];
			return link.next_ != nullptr ? link.q_ : kMinusInfinity;
		}

		[[nodiscard]] double getMaxQ() const noexcept {
			double maxQ = kMinusInfinity;
			bool found = false;
			for (const auto& link : links_) {
				if (link.next_ != nullptr && link.q_ > maxQ) {
					maxQ = link.q_;
					found = true;
				}
			}
			return found ? maxQ : kMinusInfinity;
		}

		void setQ(Direction dir, double value) noexcept {
			links_[static_cast<std::size_t>(dir)].q_ = value;
		}

		[[nodiscard]] bool isDirectionAvailable(Direction dir) const noexcept {
			return links_[static_cast<std::size_t>(dir)].next_ != nullptr;
		}

		[[nodiscard]] Cell* getNext(Direction dir) const noexcept {
			return links_[static_cast<std::size_t>(dir)].next_;
		}

		void incrementVisits() noexcept { ++visits_; }

		[[nodiscard]] std::optional<Direction> getBestDirection(double epsilon, std::mt19937& rng) const {
			std::size_t num_available = 0;
			for (std::size_t i = 0; i < links_.size(); ++i) {
				if (links_[i].next_ != nullptr) { ++num_available; }
			}

			if (num_available == 0) { return std::nullopt; }

			std::uniform_real_distribution<double> coin(0.0, 1.0);

			if (coin(rng) < epsilon) {
				std::array<Direction, kDirectionCount> available {};
				std::size_t idx = 0;
				for (std::size_t i = 0; i < links_.size(); ++i) {
					if (links_[i].next_ != nullptr) {
						available[idx++] = static_cast<Direction>(i);
					}
				}
				std::uniform_int_distribution<std::size_t> pick(0, num_available - 1);
				return available[pick(rng)];
			}

			double maxQ = kMinusInfinity;
			std::array<Direction, kDirectionCount> ties {};
			std::size_t n = 0;

			for (std::size_t i = 0; i < links_.size(); ++i) {
				if (links_[i].next_ != nullptr) {
					if (links_[i].q_ > maxQ) {
						maxQ = links_[i].q_;
						ties[0] = static_cast<Direction>(i);
						n = 1;
					} else if (links_[i].q_ == maxQ) {
						ties[n++] = static_cast<Direction>(i);
					}
				}
			}

			if (n == 1) { return ties[0]; }

			std::uniform_int_distribution<std::size_t> tieBreak(0, n - 1);
			return ties[tieBreak(rng)];
		}
	};

} // namespace lake_logic