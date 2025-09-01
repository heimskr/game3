#pragma once

#include "data/Identifier.h"
#include "threading/Lockable.h"
#include "types/Types.h"

#include <boost/json.hpp>

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace Game3 {
	class BasicBuffer;
	class Buffer;
	class Inventory;

	class ItemFilter {
		public:
			enum class Comparator: uint8_t {None = 0, Less, Greater};

			struct Config {
				boost::json::value data;
				Comparator comparator{};
				ItemCount count{};

				explicit Config(boost::json::value data = {}, Comparator comparator = {}, ItemCount count = {}):
					data(std::move(data)), comparator(comparator), count(count) {}

				bool operator()(const ItemStackPtr &, const Inventory &, bool strict) const;
				bool operator==(const Config &) const;
			};

			ItemFilter();
			ItemFilter(bool allow_mode, bool strict_);

			bool isAllowed(const ItemStackPtr &, const Inventory &) const;
			void addItem(const ItemStackPtr &);
			void removeItem(const ItemStackPtr &);
			void clear();

			void setAllowMode(bool value = true) { allowMode = value; }
			void setStrict(bool value = true) { strict = value; }

			inline bool isAllowMode() { return allowMode; }
			inline bool isStrict() { return strict; }

			inline const auto & getItems(std::shared_lock<DefaultMutex> &lock) const {
				lock = items.sharedLock();
				return items;
			}

			inline auto & getItems(std::unique_lock<DefaultMutex> &lock) {
				lock = items.uniqueLock();
				return items;
			}

			inline const auto & getConfigs(std::shared_lock<DefaultMutex> &lock) const {
				lock = configsByItem.sharedLock();
				return configsByItem;
			}

			inline auto & getConfigs(std::unique_lock<DefaultMutex> &lock) {
				lock = configsByItem.uniqueLock();
				return configsByItem;
			}

		private:
			/** If true, only the item types contained in this filter will be allowed.
			 *  If false, they will be blocked and everything else will be allowed. */
			bool allowMode = false;

			/** If false, stack data will be ignored. */
			bool strict = false;

			Lockable<std::set<Identifier>> items;
			Lockable<std::map<Identifier, std::unordered_set<Config>>> configsByItem;

		friend Buffer & operator+=(Buffer &, const ItemFilter &);
		friend Buffer & operator<<(Buffer &, const ItemFilter &);
		friend BasicBuffer & operator>>(BasicBuffer &, ItemFilter &);
	};

	using ItemFilterPtr = std::shared_ptr<ItemFilter>;

	template <typename T>
	T popBuffer(Buffer &);

	template <>
	ItemStack popBuffer<ItemStack>(Buffer &);
	Buffer & operator+=(Buffer &, const ItemFilter::Config &);
	Buffer & operator<<(Buffer &, const ItemFilter::Config &);
	BasicBuffer & operator>>(BasicBuffer &, ItemFilter::Config &);

	Buffer & operator+=(Buffer &, const ItemFilter &);
	Buffer & operator<<(Buffer &, const ItemFilter &);
	BasicBuffer & operator>>(BasicBuffer &, ItemFilter &);
}

template <>
struct std::hash<Game3::ItemFilter::Config> {
	size_t operator()(const Game3::ItemFilter::Config &config) const noexcept {
		uintmax_t hash = std::hash<Game3::ItemCount>{}(config.count);
		hash *= 0x1f1f1f1f1f1f1f1fuz;
		hash ^= std::hash<Game3::ItemFilter::Comparator>{}(config.comparator);
		hash *= 0x1f1f1f1f1f1f1f1fuz;
		hash ^= std::hash<boost::json::value>{}(config.data);
		return std::hash<uintmax_t>{}(hash);
	}
};
