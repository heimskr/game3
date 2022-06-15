#pragma once

#include <vector>

#include "game/Item.h"

namespace Game3 {
	struct CraftingRecipe {
		std::vector<ItemStack> inputs;
		std::vector<ItemStack> outputs;

		CraftingRecipe() = delete;
		CraftingRecipe(const std::vector<ItemStack> &inputs_, const std::vector<ItemStack> &outputs_): inputs(inputs_), outputs(outputs_) {}
		CraftingRecipe(std::vector<ItemStack> &&inputs_, std::vector<ItemStack> &&outputs_): inputs(std::move(inputs_)), outputs(std::move(outputs_)) {}
		CraftingRecipe(std::vector<ItemStack> &&inputs_, ItemStack &&output): inputs(std::move(inputs_)), outputs {std::move(output)} {}
	};
}
