#pragma once

#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "data/Identifier.h"
#include "registry/Registerable.h"

namespace Game3 {
	struct NamedRegistryBase;
	struct UnnamedRegistryBase;

	class Registry: public NamedRegisterable, public std::enable_shared_from_this<Registry> {
		protected:
			explicit Registry(Identifier identifier_): NamedRegisterable(std::move(identifier_)) {}

		public:
			Registry(): Registry(Identifier()) { throw std::logic_error("Cannot default-construct a Registry"); }
			virtual ~Registry() = default;

			template <typename T>
			std::shared_ptr<T> cast() {
				return std::dynamic_pointer_cast<T>(shared_from_this());
			}

			template <typename T>
			std::shared_ptr<const T> cast() const {
				return std::dynamic_pointer_cast<const T>(shared_from_this());
			}

			std::shared_ptr<NamedRegistryBase> toNamed() {
				return cast<NamedRegistryBase>();
			}

			std::shared_ptr<UnnamedRegistryBase> toUnnamed() {
				return cast<UnnamedRegistryBase>();
			}

		protected:
			size_t nextCounter = 0;
	};

	void from_json(const nlohmann::json &, Registry &);

	struct NamedRegistryBase: Registry {
		using Registry::Registry;

		virtual void add(Identifier, const nlohmann::json &) = 0;
	};

	template <typename T>
	class NamedRegistry: public NamedRegistryBase {
		static_assert(std::is_base_of_v<NamedRegisterable, T>);

		public:
			std::map<Identifier, std::shared_ptr<T>> items;
			std::vector<std::shared_ptr<T>> byCounter;

			using NamedRegistryBase::NamedRegistryBase;
			~NamedRegistry() override = default;

			NamedRegistry & operator+=(std::shared_ptr<T> item) {
				add(item);
				return *this;
			}

			NamedRegistry & operator+=(const std::pair<std::string, std::shared_ptr<T>> &pair) {
				add(pair.first, pair.second);
				return *this;
			}

			template <typename S>
			void add() {
				add(S::ID(), std::make_shared<S>());
			}

			void add(Identifier new_name, std::shared_ptr<T> new_item) {
				if (auto [iter, inserted] = items.try_emplace(std::move(new_name), std::move(new_item)); inserted) {
					iter->second->registryID = nextCounter++;
					byCounter.push_back(iter->second);
				} else
					throw std::runtime_error("NamedRegistry " + identifier.str() + " already contains an item with name \"" + new_name.str() + '"');
			}

			void add(Identifier new_name, const nlohmann::json &json) override {
				add(std::move(new_name), std::make_shared<T>(json.get<T>()));
			}

			template <typename S>
			S & get() {
				return *items.at(S::ID())->template cast<S>();
			}

			template <typename S>
			const S & get() const {
				return *items.at(S::ID())->template cast<const S>();
			}

			std::shared_ptr<T> operator[](size_t counter) const {
				return byCounter[counter];
			}

			std::shared_ptr<T> at(size_t counter) const {
				return byCounter.at(counter);
			}

			std::shared_ptr<T> operator[](const Identifier &id) const {
				return items.at(id);
			}

			std::shared_ptr<T> at(const Identifier &id) const {
				return items.at(id);
			}

			inline void clear() {
				items.clear();
				byCounter.clear();
				nextCounter = 0;
			}

			inline size_t size() const {
				return items.size();
			}
	};

	struct UnnamedRegistryBase: Registry {
		using Registry::Registry;

		virtual void add(const nlohmann::json &) = 0;
	};

	template <typename T>
	class UnnamedRegistry: public UnnamedRegistryBase {
		static_assert(std::is_base_of_v<Registerable, T>);

		public:
			std::unordered_set<std::shared_ptr<T>> items;
			std::vector<std::shared_ptr<T>> byCounter;

			using UnnamedRegistryBase::UnnamedRegistryBase;
			~UnnamedRegistry() override = default;

			UnnamedRegistry & operator+=(std::shared_ptr<T> item) {
				add(std::move(item));
				return *this;
			}

			void add(std::shared_ptr<T> item) {
				if (auto [iter, inserted] = items.insert(std::move(item)); inserted) {
					(*iter)->registryID = nextCounter++;
					byCounter.push_back(*iter);
				}
			}

			void add(const nlohmann::json &json) override {
				add(std::make_shared<T>(json));
			}

			bool addCarefully(std::shared_ptr<T> item) {
				for (const auto &existing: items)
					if (*existing == *item)
						return false;
				if (auto [iter, inserted] = items.insert(std::move(item)); inserted) {
					(*iter)->registryID = nextCounter++;
					byCounter.push_back(*iter);
					return true;
				}
				return false;
			}

			std::shared_ptr<T> operator[](size_t counter) const {
				return byCounter[counter];
			}

			std::shared_ptr<T> at(size_t counter) const {
				return byCounter.at(counter);
			}

			inline void clear() {
				items.clear();
				byCounter.clear();
				nextCounter = 0;
			}

			inline size_t size() const {
				return items.size();
			}
	};

	struct RegistryRegistry: NamedRegistry<Registry> {
		static Identifier ID() { return {"base", "registry"}; }
		RegistryRegistry(): NamedRegistry(ID()) {}
	};
}
