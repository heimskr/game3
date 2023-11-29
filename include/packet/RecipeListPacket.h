#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct CraftingRecipeRegistry;

	class RecipeListPacket: public Packet {
		public:
			static PacketID ID() { return 53; }

			Identifier recipeType;
			std::vector<nlohmann::json> recipes;

			RecipeListPacket() = default;
			RecipeListPacket(Identifier recipe_type, const CraftingRecipeRegistry &registry):
				recipeType(recipe_type), recipes(getRecipes(registry)) {}

			PacketID getID() const override { return ID(); }

			void encode(Game &, Buffer &buffer) const override { buffer << recipeType << recipes; }
			void decode(Game &, Buffer &buffer)       override { buffer >> recipeType >> recipes; }

			void handle(ClientGame &) override;

		private:
			static std::vector<nlohmann::json> getRecipes(const CraftingRecipeRegistry &);
	};
}
