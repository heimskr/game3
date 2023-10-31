#pragma once

#include "data/Identifier.h"
#include "threading/Lockable.h"

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace Game3 {
	class Buffer;
	class Item;
	class ItemStack;

	class ItemFilter {
		public:
			enum class Comparator: uint8_t {None = 0, Less, Greater};

			struct Config {
				nlohmann::json data;
				Comparator comparator{};
				ItemCount count{};

				bool operator()(const ItemStack &) const;
				bool operator<(const Config &) const;
			};

			ItemFilter();
			ItemFilter(bool allow_mode, bool strict_);

			bool isAllowed(const ItemStack &) const;
			void addItem(const ItemStack &);
			void removeItem(const ItemStack &);
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
			Lockable<std::map<Identifier, std::set<Config>>> configsByItem;

		friend Buffer & operator+=(Buffer &, const ItemFilter &);
		friend Buffer & operator<<(Buffer &, const ItemFilter &);
		friend Buffer & operator>>(Buffer &, ItemFilter &);
	};

	using ItemFilterPtr = std::shared_ptr<ItemFilter>;

	template <typename T>
	T popBuffer(Buffer &);

	template <>
	ItemStack popBuffer<ItemStack>(Buffer &);
	Buffer & operator+=(Buffer &, const ItemFilter::Config &);
	Buffer & operator<<(Buffer &, const ItemFilter::Config &);
	Buffer & operator>>(Buffer &, ItemFilter::Config &);

	Buffer & operator+=(Buffer &, const ItemFilter &);
	Buffer & operator<<(Buffer &, const ItemFilter &);
	Buffer & operator>>(Buffer &, ItemFilter &);
}
