#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

namespace Game3 {
	class Registry {
		public:
			std::string name;

			Registry(std::string name_): name(std::move(name_)) {}

			virtual ~Registry() = 0;
	};

	template <typename T>
	class NamedRegistry: public Registry {
		public:
			std::map<std::string, std::shared_ptr<T>> items;

			using Registry::Registry;
			~NamedRegistry() override = default;

			NamedRegistry & operator+=(const std::pair<std::string, std::shared_ptr<T>> &pair) {
				if (items.contains(pair.first))
					throw std::runtime_error("NamedRegistry " + name + " already contains an item with name \"" + pair.first + '"');
				items.emplace(pair);
				return *this;
			}
	};

	template <typename T>
	class UnnamedRegistry: public Registry {
		public:
			std::unordered_set<std::shared_ptr<T>> items;

			using Registry::Registry;
			~UnnamedRegistry() override = default;

			UnnamedRegistry & operator+=(std::shared_ptr<T> item) {
				items.insert(std::move(item));
			}

			bool insertCarefully(std::shared_ptr<T> item) {
				for (const auto &existing: items)
					if (*existing == *item)
						return false;
				items.insert(std::move(item));
				return true;
			}
	};
}
