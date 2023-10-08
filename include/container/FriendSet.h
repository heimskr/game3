#include <cassert>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Game3 {
	/** A data structure containing multiple sets of related items. Sets can be found given one of the members. */
	template <typename T, template <typename...> typename M = std::unordered_map, template <typename...> typename S = std::unordered_set, typename N = size_t>
	class FriendSet {
		private:
			M<T, N> indices;
			M<N, S<T>> sets;
			S<N> freePool;
			N nextIndex = 0;

			N getIndex() {
				if (!freePool.empty()) {
					const auto iter = freePool.begin();
					N out = *iter;
					freePool.erase(iter);
					return out;
				}

				return nextIndex++;
			}

			auto insertNew(const T &value) {
				const N index = getIndex();
				const auto [indices_iter, indices_inserted] = indices.emplace(value, index);
				assert(indices_inserted);

				const auto [sets_iter, sets_inserted] = sets.emplace(index, S{value});
				assert(sets_inserted);

				return sets_iter;
			}

		public:
			FriendSet() = default;

			/** Inserts a friendless item into a new subset. */
			std::pair<typename decltype(sets)::iterator, bool> insert(const T &value) {
				if (const auto index_iter = indices.find(value); index_iter != indices.end()) {
					const auto sets_iter = sets.find(index_iter->second);
					assert(sets_iter != sets.end());
					return {sets_iter, false};
				}

				return {insertNew(value), true};
			}

			/** Inserts an item into the friend set next to a friend if possible, or in a new subset otherwise. */
			std::pair<typename decltype(sets)::iterator, bool> insert(const T &value, const T &buddy) {
				if (const auto index_iter = indices.find(value); index_iter != indices.end()) {
					const auto sets_iter = sets.find(index_iter->second);
					assert(sets_iter != sets.end());
					return {sets_iter, false};
				}

				if (const auto index_iter = indices.find(buddy); index_iter != indices.end()) {
					const auto sets_iter = sets.find(index_iter->second);
					assert(sets_iter != sets.end());
					const auto [iter, inserted] = sets_iter->second.insert(value);
					assert(inserted);
					return {sets_iter, true};
				}

				return {insertNew(value), true};
			}

			/** Returns a coinst reference to the set that the value is a member of.
			 *  Throws std::out_of_range if the value is not a member of any subset. */
			const S<N> & at(const T &value) const {
				if (const auto index_iter = indices.find(value); index_iter != indices.end())
					return sets.at(index_iter->second);
				throw std::out_of_range("FriendSet::at");
			}

			/** Returns a const reference to the set that the value is a member of.
			 *  If the value is not a member of any subset, it will be inserted into a new one. */
			const S<N> & operator[](const T &value) {
				if (const auto index_iter = indices.find(value); index_iter != indices.end())
					return sets.at(index_iter->second);
				return insertNew(value)->second;
			}

			/** Erases a value from its subset if it's part of one.
			 *  Returns whether the value was present and erased. */
			bool erase(const T &value) {
				if (const auto index_iter = indices.find(value); index_iter != indices.end()) {
					const N index = index_iter->second;
					indices.erase(index_iter);
					auto set_iter = sets.find(index);
					assert(set_iter != sets.end());
					S<T> &set = set_iter->second;
					if (set.size() == 1) {
						sets.erase(set_iter);
						freePool.insert(index);
					} else
						set.erase(value);
					return true;
				}

				return false;
			}

			/** Erases the entire subset the given value belongs to.
			 *  Returns the number of items erased. */
			size_t eraseSet(const T &value) {
				if (const auto index_iter = indices.find(value); index_iter != indices.end()) {
					const N index = index_iter->second;

					auto set_iter = sets.find(index);
					assert(set_iter != sets.end());

					S<T> &set = set_iter->second;
					assert(!set.empty());
					const size_t out = set.size();

					for (auto iter = set.begin(); iter != set.end();) {
						indices.erase(*iter);
						set.erase(iter++);
					}

					freePool.insert(index);
					return out;
				}

				return 0;
			}

			auto begin() { return sets.begin(); }
			auto end()   { return sets.end(); }
			auto begin() const { return sets.begin(); }
			auto end()   const { return sets.end();   }
	};
}
