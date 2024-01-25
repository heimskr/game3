#pragma once

// Inspired by https://stackoverflow.com/a/51973872

#include <random>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace Game3 {
	template <typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
	class RandomSet {
		private:
			std::unordered_map<T, size_t, Hash, Equal> map;
			using map_iterator = decltype(map)::iterator;
			std::vector<map_iterator> iterators;

		public:
			class iterator {
				public:
					iterator() = delete;

					explicit iterator(map_iterator mapped_):
						mapped(std::move(mapped_)) {}

					bool operator==(const iterator &other) const {
						return this == &other || mapped == other.mapped;
					}

					bool operator!=(const iterator &other) const {
						return this != &other && mapped != other.mapped;
					}

					iterator & operator++() {
						++mapped;
						return *this;
					}

					iterator operator++(int) {
						iterator old(*this);
						++mapped;
						return old;
					}

					iterator & operator--() {
						--mapped;
						return *this;
					}

					iterator operator--(int) {
						iterator old(*this);
						--mapped;
						return old;
					}

					const T & operator*() const {
						return mapped->first;
					}

					const T * operator->() const {
						return &mapped->first;
					}

				private:
					map_iterator mapped;
			};

			RandomSet() = default;

			template <typename Iter>
			RandomSet(Iter begin, Iter end) {
				insert(begin, end);
			}

			template <typename Iter>
			void insert(Iter begin, Iter end) {
				for (; begin != end; ++begin)
					insert(*begin);
			}

			void contains(const T &key) {
				return map.contains(key);
			}

			std::pair<iterator, bool> insert(const T &item) {
				auto [iter, inserted] = map.emplace(item, iterators.size());

				if (inserted)
					iterators.push_back(iter);

				return {iterator(iter), inserted};
			}

			std::pair<iterator, bool> insert(T &&item) {
				auto [iter, inserted] = map.emplace(std::move(item), iterators.size());

				if (inserted)
					iterators.push_back(iter);

				return {iterator(iter), inserted};
			}

			size_t erase(const T &item) {
				return erase(find(item));
			}

			size_t erase(iterator iter) {
				if (iter.mapped == map.end())
					return 0;

				const size_t back_index = iterators.size() - 1;
				const size_t old_index = iter.mapped->second;

				if (old_index == back_index) {
					map.erase(iter.mapped);
					iterators.pop_back();
					return 1;
				}

				map_iterator &other_iter = iterators.back();
				other_iter->second = old_index;
				std::swap(iterators.at(old_index), other_iter);
				iterators.pop_back();
				map.erase(iter.mapped);
				return 1;
			}

			bool empty() const {
				return iterators.empty();
			}

			size_t size() const {
				return iterators.size();
			}

			template <typename R>
			T & choose(R &rng) {
				if (empty())
					throw std::out_of_range("RandomSet is empty");

				return iterators.at(std::uniform_int_distribution(0, size() - 1)(rng))->first;
			}

			template <typename R>
			const T & choose(R &rng) const {
				if (empty())
					throw std::out_of_range("RandomSet is empty");

				return iterators.at(std::uniform_int_distribution(0, size() - 1)(rng))->first;
			}

			iterator find(const T &key) const {
				return iterator(map.find(key));
			}

			iterator begin() const {
				return iterator(map.begin());
			}

			iterator end() const {
				return iterator(map.end());
			}

			void clear() {
				map.clear();
				iterators.clear();
			}

			template <typename... Args>
			std::pair<iterator, bool> emplace(Args &&...args) {
				// Not really in the spirit of emplace, but whatever.
				// Especially because you can't use this method to insert something with no move constructor...
				// Though maybe map keys are required to be movable anyway?
				return insert(T(std::forward<Args>(args)...));
			}
	};
}
