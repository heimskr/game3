#pragma once

#include "Direction.h"

namespace Game3 {
	template <typename T>
	class DirectionalContainer {
		private:
			std::optional<T> north;
			std::optional<T> east;
			std::optional<T> south;
			std::optional<T> west;

			template <typename P>
			struct iterator_base {
				private:
					using Member = std::optional<T> DirectionalContainer::*;
					P &parent;
					Member member;

					static Member next(Member start) {
						if (start == &DirectionalContainer::north) return &DirectionalContainer::east;
						if (start == &DirectionalContainer::east)  return &DirectionalContainer::south;
						if (start == &DirectionalContainer::south) return &DirectionalContainer::west;
						return nullptr;
					}

					bool valid() const {
						return member && (parent.*member).has_value();
					}

				public:
					explicit iterator_base(P &parent_, Member member_ = &DirectionalContainer::north):
						parent(parent_), member(member_) {}

					iterator_base<P> & operator++() {
						Member next_member = next(member);
						while (next_member && !(parent.*next_member).has_value())
							next_member = next(next_member);
						member = next_member;
						return *this;
					}

					iterator_base<P> operator++(int) {
						Member old_member = member;
						++*this;
						return {parent, old_member};
					}

					T & operator*() {
						return (parent.*member).value();
					}

					const T & operator*() const {
						return (parent.*member).value();
					}

					bool operator==(const iterator_base<P> &other) const {
						return this == &other || (&parent == &other.parent && (member == other.member || (!valid() && !other.valid())));
					}
			};

		// Sorry for all the duplication.
		public:
			using iterator = iterator_base<DirectionalContainer>;
			using const_iterator = iterator_base<const DirectionalContainer>;

			inline iterator begin() {
				return iterator(*this);
			}

			inline const_iterator begin() const {
				return const_iterator(*this);
			}

			inline iterator end() {
				return iterator(*this, nullptr);
			}

			inline const_iterator end() const {
				return const_iterator(*this, nullptr);
			}

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
