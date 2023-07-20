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

			template <typename P>
			struct iterator_base {
				private:
					using Member = std::optional<T> DirectionalContainer<T>::*;
					P &parent;
					Member member;
					bool isEnd = false;

					static Member next(Member start) {
						if (start == &DirectionalContainer<T>::north) return &DirectionalContainer<T>::east;
						if (start == &DirectionalContainer<T>::east)  return &DirectionalContainer<T>::south;
						if (start == &DirectionalContainer<T>::south) return &DirectionalContainer<T>::west;
						return nullptr;
					}

					static std::string mstr(Member member) {
						return (member == &DirectionalContainer::north)? "north" : (member == &DirectionalContainer::east)? "east" :
						       (member == &DirectionalContainer::south)? "south" : (member == &DirectionalContainer::west)? "west" : "???";
					}

				public:
					explicit iterator_base(P &parent_, Member member_ = &DirectionalContainer<T>::north):
						parent(parent_), member(member_) {}

					iterator_base(P &parent_, int):
						parent(parent_), member(nullptr), isEnd(true) {}

					inline iterator_base<P> & operator++() {
						std::cerr << "++()\n";
						Member next_member = next(member);
						while (next_member && !(parent.*next_member).has_value())
							next_member = next(next_member);
						std::cerr << mstr(member) << " -> " << mstr(next_member) << '\n';
						member = next_member;
						return *this;
					}

					inline iterator_base<P> operator++(int) {
						INFO("()++");
						Member old_member = member;
						++*this;
						return {parent, old_member};
					}

					inline T & operator*() {
						assert(member);
						return (parent.*member).value();
					}

					inline const T & operator*() const {
						assert(member);
						return (parent.*member).value();
					}

					bool operator==(const iterator_base<P> &other) const {
						// What
						return this == &other || (&parent == &other.parent && (member == other.member || (isEnd && (other.member == nullptr || other.isEnd)) || ((member == nullptr || isEnd) && other.isEnd)));
					}
			};

		// Sorry for all the duplication.
		public:
			using iterator = iterator_base<DirectionalContainer<T>>;
			using const_iterator = iterator_base<const DirectionalContainer<T>>;

			inline iterator begin() {
				return iterator(*this);
			}

			inline const_iterator begin() const {
				return const_iterator(*this);
			}

			inline iterator end() {
				return iterator(*this, 1);
			}

			inline const_iterator end() const {
				return const_iterator(*this, 1);
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
