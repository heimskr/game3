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
			std::vector<boost::json::value> recipes;

			RecipeListPacket() = default;
			RecipeListPacket(Identifier recipe_type, const CraftingRecipeRegistry &registry, const GamePtr &game):
				recipeType(recipe_type), recipes(getRecipes(registry, game)) {}

			PacketID getID() const override { return ID(); }

			void encode(Game &, Buffer &buffer) const override { buffer << recipeType << recipes; }
			void decode(Game &, Buffer &buffer)       override { buffer >> recipeType >> recipes; }

			void handle(const std::shared_ptr<ClientGame> &) override;

		private:
			static std::vector<boost::json::value> getRecipes(const CraftingRecipeRegistry &, const GamePtr &);
	};
}
