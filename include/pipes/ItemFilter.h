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

			inline const auto & getData(std::shared_lock<DefaultMutex> &lock) const {
				lock = dataByItem.sharedLock();
				return dataByItem;
			}

			inline auto & getData(std::unique_lock<DefaultMutex> &lock) {
				lock = dataByItem.uniqueLock();
				return dataByItem;
			}

		private:
			/** If true, only the item types contained in this filter will be allowed.
			 *  If false, they will be blocked and everything else will be allowed. */
			bool allowMode = false;

			/** If false, stack data will be ignored. */
			bool strict = false;

			Lockable<std::set<Identifier>> items;
			Lockable<std::map<Identifier, std::set<nlohmann::json>>> dataByItem;

		friend Buffer & operator+=(Buffer &, const ItemFilter &);
		friend Buffer & operator<<(Buffer &, const ItemFilter &);
		friend Buffer & operator>>(Buffer &, ItemFilter &);
	};

	Buffer & operator+=(Buffer &, const ItemFilter &);
	Buffer & operator<<(Buffer &, const ItemFilter &);
	Buffer & operator>>(Buffer &, ItemFilter &);
}
