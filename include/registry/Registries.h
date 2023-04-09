#pragma once

#include "Types.h"
#include "registry/Registry.h"

namespace Game3 {
	class Item;
	class Texture;
	class Tileset;
	struct CraftingRecipe;
	struct EntityFactory;
	struct EntityTexture;
	struct GhostDetails;
	struct ItemTexture;

	struct DurabilityRegistry: NamedRegistry<Durability> {
		static Identifier ID() { return {"base", "durability"}; }
		DurabilityRegistry(): NamedRegistry(ID()) {}
	};

	struct ItemRegistry: NamedRegistry<Item> {
		static Identifier ID() { return {"base", "item"}; }
		ItemRegistry(): NamedRegistry(ID()) {}
	};

	struct CraftingRecipeRegistry: UnnamedRegistry<CraftingRecipe> {
		static Identifier ID() { return {"base", "crafting_recipe"}; }
		CraftingRecipeRegistry(): UnnamedRegistry(ID()) {}
	};

	struct ItemTextureRegistry: NamedRegistry<ItemTexture> {
		static Identifier ID() { return {"base", "item_texture"}; }
		ItemTextureRegistry(): NamedRegistry(ID()) {}
	};

	struct TextureRegistry: NamedRegistry<Texture> {
		static Identifier ID() { return {"base", "texture"}; }
		TextureRegistry(): NamedRegistry(ID()) {}
	};

	struct EntityTextureRegistry: NamedRegistry<EntityTexture> {
		static Identifier ID() { return {"base", "entity_texture"}; }
		EntityTextureRegistry(): NamedRegistry(ID()) {}
	};

	struct EntityFactoryRegistry: NamedRegistry<EntityFactory> {
		static Identifier ID() { return {"base", "entity_factory"}; }
		EntityFactoryRegistry(): NamedRegistry(ID()) {}
	};

	struct TilesetRegistry: NamedRegistry<Tileset> {
		static Identifier ID() { return {"base", "tileset"}; }
		TilesetRegistry(): NamedRegistry(ID()) {}
	};

	struct GhostDetailsRegistry: NamedRegistry<GhostDetails> {
		static Identifier ID() { return {"base", "ghost_details"}; }
		GhostDetailsRegistry(): NamedRegistry(ID()) {}
	};
}
