#pragma once

#include <variant>

#include <nlohmann/json_fwd.hpp>

#include "item/Item.h"

namespace Game3 {
	struct AttributeRequirement {
		Identifier attribute;
		ItemCount count = 1;
	};

	struct CraftingRequirement: std::variant<ItemStack, AttributeRequirement> {
		using std::variant<ItemStack, AttributeRequirement>::variant;

		static CraftingRequirement fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);

		template <typename T>
		constexpr inline bool is() const {
			return std::holds_alternative<T>(*this);
		}

		template <typename T>
		constexpr inline T & get() {
			return std::get<T>(*this);
		}

		template <typename T>
		constexpr inline const T & get() const {
			return std::get<T>(*this);
		}

		template <typename T>
		constexpr inline T * get_if() {
			return std::get_if<T>(this);
		}

		template <typename T>
		constexpr inline const T * get_if() const {
			return std::get_if<T>(this);
		}

		constexpr inline ItemCount count() const {
			return is<ItemStack>()? get<ItemStack>().count : get<AttributeRequirement>().count;
		}
	};

	void to_json(nlohmann::json &, const CraftingRequirement &);
}
