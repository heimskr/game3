#pragma once

#include "Direction.h"
#include "Log.h"

namespace Game3 {
	template <typename T>
	class DirectionalContainer {
		private:
			std::optional<T> north;
			std::optional<T> east;
			std::optional<T> south;
			std::optional<T> west;

		// Sorry for all the duplication.
		public:
			inline T & operator[](Direction direction) {
				std::optional<T> *item = nullptr;
				switch (direction) {
					case Direction::Up:    item = &north; break;
					case Direction::Right: item = &east;  break;
					case Direction::Down:  item = &south; break;
					case Direction::Left:  item = &west;  break;
					default:
						throw std::out_of_range("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
				}

				if (!*item)
					item->emplace();

				return **item;
			}

			const T & operator[](Direction direction) const {
				const std::optional<T> *item = nullptr;
				switch (direction) {
					case Direction::Up:    item = &north; break;
					case Direction::Right: item = &east;  break;
					case Direction::Down:  item = &south; break;
					case Direction::Left:  item = &west;  break;
					default:
						throw std::out_of_range("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
				}

				if (!*item)
					item->emplace();

				return **item;
			}

			inline T & at(Direction direction) {
				std::optional<T> *item = nullptr;
				switch (direction) {
					case Direction::Up:    item = &north; break;
					case Direction::Right: item = &east;  break;
					case Direction::Down:  item = &south; break;
					case Direction::Left:  item = &west;  break;
					default:
						throw std::out_of_range("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
				}

				if (!*item)
					throw std::out_of_range("Direction " + toString(direction) + " not contained");

				return **item;
			}

			inline const T & at(Direction direction) const {
				const std::optional<T> *item = nullptr;
				switch (direction) {
					case Direction::Up:    item = &north; break;
					case Direction::Right: item = &east;  break;
					case Direction::Down:  item = &south; break;
					case Direction::Left:  item = &west;  break;
					default:
						throw std::out_of_range("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
				}

				if (!*item)
					throw std::out_of_range("Direction " + toString(direction) + " not contained");

				return **item;
			}
	};
}
