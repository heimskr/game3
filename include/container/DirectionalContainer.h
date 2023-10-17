#pragma once

#include "types/Direction.h"
#include "Log.h"
#include "net/Buffer.h"

namespace Game3 {
	class Buffer;

	template <typename T>
	class DirectionalContainer {
		public:
			std::optional<T> north;
			std::optional<T> east;
			std::optional<T> south;
			std::optional<T> west;

			// Sorry for all the duplication.
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

	template <typename T>
	Buffer & operator+=(Buffer &buffer, const DirectionalContainer<T> &container) {
		return buffer << container;
	}

	template <typename T>
	Buffer & operator<<(Buffer &buffer, const DirectionalContainer<T> &container) {
		return buffer << container.north << container.east << container.south << container.west;
	}

	template <typename T>
	Buffer & operator>>(Buffer &buffer, DirectionalContainer<T> &container) {
		return buffer >> container.north >> container.east >> container.south >> container.west;
	}
}
