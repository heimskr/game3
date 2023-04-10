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
	struct GhostFunction;
	struct ItemTexture;
	struct TileEntityFactory;

	struct RegistryRegistry: NamedRegistry<Registry> {
		static Identifier ID() { return {"base", "registry"}; }
		RegistryRegistry(): NamedRegistry(ID()) {}
	};

	struct DurabilityRegistry: NamedRegistry<NamedDurability> {
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

	struct GhostFunctionRegistry: NamedRegistry<GhostFunction> {
		static Identifier ID() { return {"base", "ghost_function"}; }
		GhostFunctionRegistry(): NamedRegistry(ID()) {}
	};

	struct TileEntityFactoryRegistry: NamedRegistry<TileEntityFactory> {
		static Identifier ID() { return {"base", "tile_entity_factory"}; }
		TileEntityFactoryRegistry(): NamedRegistry(ID()) {}
	};
}
