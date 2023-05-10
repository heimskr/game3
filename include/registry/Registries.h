#pragma once

#include "Types.h"
#include "registry/Registry.h"

namespace Game3 {
	class EntityFactory;
	class GhostFunction;
	class Item;
	class PacketFactory;
	class RealmFactory;
	class Texture;
	class Tileset;
	class TileEntityFactory;
	struct CraftingRecipe;
	struct EntityTexture;
	struct GhostDetails;
	struct ItemTexture;
	struct Ore;
	struct RealmDetails;

	struct RegistryRegistry: NamedRegistry<Registry> {
		static Identifier ID() { return {"base", "registry"}; }
		RegistryRegistry(): NamedRegistry(ID()) {}
	};

	struct ItemRegistry: NamedRegistry<Item> {
		static Identifier ID() { return {"base", "item"}; }
		ItemRegistry(): NamedRegistry(ID()) {}
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

	struct OreRegistry: NamedRegistry<Ore> {
		static Identifier ID() { return {"base", "ore"}; }
		OreRegistry(): NamedRegistry(ID()) {}
	};

	struct RealmFactoryRegistry: NamedRegistry<RealmFactory> {
		static Identifier ID() { return {"base", "realm_factory"}; }
		RealmFactoryRegistry(): NamedRegistry(ID()) {}
	};

	struct RealmTypeRegistry: IdentifierRegistry {
		static Identifier ID() { return {"base", "realm_type"}; }
		RealmTypeRegistry(): IdentifierRegistry(ID()) {}
	};

	struct RealmDetailsRegistry: NamedRegistry<RealmDetails> {
		static Identifier ID() { return {"base", "realm_details"}; }
		RealmDetailsRegistry(): NamedRegistry(ID()) {}
	};

	struct PacketFactoryRegistry: NumericRegistry<PacketFactory> {
		static Identifier ID() { return {"base", "packet_factory"}; }
		PacketFactoryRegistry(): NumericRegistry(ID()) {}
	};
}
