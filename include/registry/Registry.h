#pragma once

#include "Log.h"
#include "data/Identifier.h"
#include "registry/Registerable.h"
#include "threading/HasMutex.h"

#include <boost/json.hpp>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Game3 {
	class Game;
	struct NamedRegistryBase;
	struct UnnamedRegistryBase;

	class Registry: public NamedRegisterable, public HasMutex<>, public std::enable_shared_from_this<Registry> {
		protected:
			explicit Registry(Identifier identifier_): NamedRegisterable(std::move(identifier_)) {}

		public:
			Registry(): Registry(Identifier()) { throw std::logic_error("Cannot default-construct a Registry"); }
			virtual ~Registry() = default;

			std::shared_ptr<NamedRegistryBase> toNamed() {
				return std::dynamic_pointer_cast<NamedRegistryBase>(shared_from_this());
			}

			std::shared_ptr<UnnamedRegistryBase> toUnnamed() {
				return std::dynamic_pointer_cast<UnnamedRegistryBase>(shared_from_this());
			}

		protected:
			size_t nextCounter = 0;
	};

	class IdentifierRegistry: public Registry {
		public:
			using Registry::Registry;

			std::set<Identifier> items;

			IdentifierRegistry & operator+=(Identifier item) {
				add(std::move(item));
				return *this;
			}

			template <typename S>
			void add() {
				add(S::ID());
			}

			void add(Identifier item) {
				items.insert(std::move(item));
			}

			template <typename S>
			const Identifier & contains() const {
				return items.contains(S::ID());
			}

			inline bool contains(const Identifier &id) {
				return items.contains(id);
			}

			inline void clear() {
				items.clear();
			}

			inline size_t size() const {
				return items.size();
			}

			inline auto begin() const {
				return items.begin();
			}

			inline auto end() const {
				return items.end();
			}

			inline auto rbegin() const {
				return items.rbegin();
			}

			inline auto rend() const {
				return items.rend();
			}
	};

	struct NamedRegistryBase: Registry {
		using Registry::Registry;
	};

	template <typename T>
	class NamedRegistry: public NamedRegistryBase {
#ifdef REGISTRY_ASSERTS
		static_assert(std::is_base_of_v<NamedRegisterable, T>);
#endif

		public:
			std::map<Identifier, std::shared_ptr<T>> items;
			std::vector<std::shared_ptr<T>> byCounter;

			using NamedRegistryBase::NamedRegistryBase;
			~NamedRegistry() override = default;

			inline NamedRegistry & operator+=(std::shared_ptr<T> item) {
				add(item);
				return *this;
			}

			inline NamedRegistry & operator+=(const std::pair<std::string, std::shared_ptr<T>> &pair) {
				add(pair.first, pair.second);
				return *this;
			}

			template <typename S>
			inline auto add() {
				auto ptr = std::make_shared<S>();
				add(S::ID(), ptr);
				return ptr;
			}

			template <typename S>
			inline auto add(Identifier new_name) {
				auto ptr = std::make_shared<S>(new_name);
				return add(std::move(new_name), std::move(ptr));
			}

			inline std::shared_ptr<T> add(Identifier new_name, std::shared_ptr<T> new_item) {
				if (auto [iter, inserted] = items.try_emplace(new_name, std::move(new_item)); inserted) {
					iter->second->identifier = std::move(new_name);
					iter->second->registryID = nextCounter++;
					byCounter.push_back(iter->second);
					return iter->second;
				}

				throw std::runtime_error("NamedRegistry " + identifier.str() + " already contains an item with name \"" + new_name.str() + '"');
			}

			inline std::shared_ptr<T> add(Identifier new_name, T &&new_item) {
				return add(std::move(new_name), std::make_shared<T>(std::move(new_item)));
			}

			inline bool contains(const Identifier &id) const {
				return items.contains(id);
			}

			template <typename S>
			inline S & get() {
				return *std::dynamic_pointer_cast<S>(items.at(S::ID()));
			}

			template <typename S>
			inline const S & get() const {
				return *std::dynamic_pointer_cast<const S>(items.at(S::ID()));
			}

			inline std::shared_ptr<T> maybe(const Identifier &id) const {
				auto iter = items.find(id);
				if (iter == items.end()) {
					return {};
				}
				return iter->second;
			}

			inline std::shared_ptr<T> maybe(size_t counter) const {
				if (counter < byCounter.size()) {
					return byCounter[counter];
				}
				return {};
			}

			inline const std::shared_ptr<T> & operator[](size_t counter) const {
				return at(counter);
			}

			inline const std::shared_ptr<T> & at(size_t counter) const {
				return byCounter.at(counter);
			}

			inline const std::shared_ptr<T> & operator[](const Identifier &id) const {
				return at(id);
			}

			inline const std::shared_ptr<T> & at(const Identifier &id) const {
				return items.at(id);
			}

			inline std::shared_ptr<T> & operator[](size_t counter) {
				return at(counter);
			}

			inline std::shared_ptr<T> & at(size_t counter) {
				return byCounter.at(counter);
			}

			inline std::shared_ptr<T> & operator[](const Identifier &id) {
				return at(id);
			}

			inline std::shared_ptr<T> & at(const Identifier &id) {
				return items.at(id);
			}

			inline auto find(const Identifier &id) const {
				return items.find(id);
			}

			inline auto find(const Identifier &id) {
				return items.find(id);
			}

			inline void clear() {
				items.clear();
				byCounter.clear();
				nextCounter = 0;
			}

			inline size_t size() const {
				return items.size();
			}

			inline auto begin() const {
				return items.begin();
			}

			inline auto end() const {
				return items.end();
			}

			inline auto rbegin() const {
				return items.rbegin();
			}

			inline auto rend() const {
				return items.rend();
			}
	};

	struct UnnamedRegistryBase: Registry {
		using Registry::Registry;

		virtual void add(const std::shared_ptr<Game> &, const boost::json::value &) = 0;
	};

	template <typename T, template <typename...> typename Set = std::unordered_set>
	class UnnamedRegistry: public UnnamedRegistryBase {
#ifdef REGISTRY_ASSERTS
		static_assert(std::is_base_of_v<Registerable, T>);
#endif

		public:
			Set<std::shared_ptr<T>> items;
			std::vector<std::shared_ptr<T>> byCounter;

			using UnnamedRegistryBase::UnnamedRegistryBase;
			~UnnamedRegistry() override = default;

			virtual void onAdd(const T &) {}

			inline UnnamedRegistry & operator+=(std::shared_ptr<T> item) {
				add(std::move(item));
				return *this;
			}

			inline void add(std::shared_ptr<T> item) {
				if (auto [iter, inserted] = items.insert(std::move(item)); inserted) {
					(*iter)->registryID = nextCounter++;
					byCounter.push_back(*iter);
					onAdd(**iter);
				}
			}

			inline void add(T &&item) {
				add(std::make_shared<T>(std::move(item)));
			}

			bool addCarefully(std::shared_ptr<T> item) {
				for (const auto &existing: items)
					if (*existing == *item)
						return false;
				if (auto [iter, inserted] = items.insert(std::move(item)); inserted) {
					(*iter)->registryID = nextCounter++;
					byCounter.push_back(*iter);
					onAdd(**iter);
					return true;
				}
				return false;
			}

			void add(const std::shared_ptr<Game> &, const boost::json::value &) override {
				throw std::runtime_error("Adding from JSON unimplemented");
			}

			inline std::shared_ptr<T> maybe(size_t counter) {
				if (counter < byCounter.size())
					return byCounter[counter];
				return {};
			}

			inline std::shared_ptr<T> operator[](size_t counter) const {
				return at(counter);
			}

			inline std::shared_ptr<T> at(size_t counter) const {
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

			inline bool empty() const {
				return items.empty();
			}

			inline auto begin() const {
				return byCounter.begin();
			}

			inline auto end() const {
				return byCounter.end();
			}

			inline auto rbegin() const {
				return byCounter.rbegin();
			}

			inline auto rend() const {
				return byCounter.rend();
			}
	};

	template <typename T, template <typename...> typename Set = std::unordered_set>
	class UnnamedJSONRegistry: public UnnamedRegistry<T, Set> {
		public:
			using UnnamedRegistry<T, Set>::UnnamedRegistry;

			void add(const std::shared_ptr<Game> &game, const boost::json::value &json) override {
				UnnamedRegistry<T, Set>::add(boost::json::value_to<T>(json, game));
			}
	};

	struct NumericRegistryBase: Registry {
		using Registry::Registry;
	};

	template <typename T>
	class NumericRegistry: public NumericRegistryBase {
#ifdef REGISTRY_ASSERTS
		static_assert(std::is_base_of_v<NumericRegisterable, T>);
#endif

		public:
			std::map<NumericRegisterable::Type, std::shared_ptr<T>> items;

			using NumericRegistryBase::NumericRegistryBase;
			~NumericRegistry() override = default;

			inline NumericRegistry & operator+=(std::shared_ptr<T> item) {
				add(item);
				return *this;
			}

			inline NumericRegistry & operator+=(const std::pair<NumericRegisterable::Type, std::shared_ptr<T>> &pair) {
				add(pair.first, pair.second);
				return *this;
			}

			inline void add(NumericRegisterable::Type new_number, std::shared_ptr<T> new_item) {
				if (auto [iter, inserted] = items.try_emplace(new_number, std::move(new_item)); inserted) {
					iter->second->number = new_number;
				} else
					throw std::runtime_error("NumericRegistry " + identifier.str() + " already contains an item with number " + std::to_string(new_number));
			}

			inline void add(const std::shared_ptr<T> &new_item) {
				add(new_item->number, new_item);
			}

			inline void add(NumericRegisterable::Type new_number, T &&new_item) {
				add(new_number, std::make_shared<T>(std::move(new_item)));
			}

			inline bool contains(NumericRegisterable::Type number) const {
				return items.contains(number);
			}

			inline std::shared_ptr<T> operator[](NumericRegisterable::Type number) const {
				return at(number);
			}

			inline std::shared_ptr<T> at(NumericRegisterable::Type number) const {
				try {
					return items.at(number);
				} catch (const std::out_of_range &) {
					ERROR("Couldn't find \"{}\" in registry {}", number, identifier);
					return {};
				}
			}

			inline void clear() {
				items.clear();
			}

			inline size_t size() const {
				return items.size();
			}
	};

	struct StringRegistryBase: Registry {
		using Registry::Registry;
	};

	template <typename T>
	class StringRegistry: public StringRegistryBase {
#ifdef REGISTRY_ASSERTS
		static_assert(std::is_base_of_v<StringRegisterable, T>);
#endif

		public:
			std::map<std::string, std::shared_ptr<T>> items;

			using StringRegistryBase::StringRegistryBase;
			~StringRegistry() override = default;

			inline StringRegistry & operator+=(std::shared_ptr<T> item) {
				add(item);
				return *this;
			}

			inline StringRegistry & operator+=(const std::pair<std::string, std::shared_ptr<T>> &pair) {
				add(pair.first, pair.second);
				return *this;
			}

			inline void add(const std::string &new_name, std::shared_ptr<T> new_item) {
				if (auto [iter, inserted] = items.try_emplace(new_name, std::move(new_item)); inserted) {
					iter->second->name = new_name;
				} else
					throw std::runtime_error("StringRegistry " + identifier.str() + " already contains an item with name " + new_name);
			}

			inline void add(const std::shared_ptr<T> &new_item) {
				add(new_item->name, new_item);
			}

			inline void add(const std::string &new_name, T &&new_item) {
				add(new_name, std::make_shared<T>(std::move(new_item)));
			}

			inline bool contains(const std::string &name) const {
				return items.contains(name);
			}

			inline std::shared_ptr<T> maybe(const std::string &name) {
				auto iter = items.find(name);
				if (iter == items.end())
					return {};
				return iter->second;
			}

			inline std::shared_ptr<T> operator[](const std::string &name) const {
				return at(name);
			}

			inline std::shared_ptr<T> at(const std::string &name) const {
				try {
					return items.at(name);
				} catch (const std::out_of_range &) {
					ERROR("Couldn't find \"{}\" in registry {}", name, identifier);
					return {};
				}
			}

			inline void clear() {
				items.clear();
			}

			inline size_t size() const {
				return items.size();
			}
	};

	template <typename T>
	class NamedJSONRegistry: public StringRegistry<T> {
		public:
			using StringRegistry<T>::StringRegistry;

			void add(const std::shared_ptr<Game> &game, const std::string &name, const boost::json::value &json) {
				StringRegistry<T>::add(name, boost::json::value_to<T>(json, game));
			}
	};
}
