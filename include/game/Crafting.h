// #pragma once

// #include <vector>

// #include "item/Item.h"

// namespace Game3 {
// 	struct CraftingRecipe {
// 		std::vector<ItemStack> inputs;
// 		std::vector<ItemStack> outputs;
// 		CraftingStationType station = CraftingStationType::None;

// 		CraftingRecipe() = delete;

// 		CraftingRecipe(const std::vector<ItemStack> &inputs_, const std::vector<ItemStack> &outputs_, CraftingStationType station_ = CraftingStationType::None):
// 			inputs(inputs_), outputs(outputs_), station(station_) {}

// 		CraftingRecipe(std::vector<ItemStack> &&inputs_, std::vector<ItemStack> &&outputs_, CraftingStationType station_ = CraftingStationType::None):
// 			inputs(std::move(inputs_)), outputs(std::move(outputs_)), station(station_) {}

// 		CraftingRecipe(std::vector<ItemStack> &&inputs_, ItemStack &&output, CraftingStationType station_ = CraftingStationType::None):
// 			inputs(std::move(inputs_)), outputs {std::move(output)}, station(station_) {}
// 	};
// }
