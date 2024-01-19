#pragma once

#include "types/Types.h"
#include "registry/Registry.h"

namespace Game3 {
	class Crop;
	class EntityFactory;
	class Item;
	class ItemSet;
	class ItemStack;
	class ItemTexture;
	class LocalCommandFactory;
	class ModuleFactory;
	class PacketFactory;
	class RealmFactory;
	class Resource;
	class Texture;
	class Tile;
	class Tileset;
	class TileEntityFactory;
	struct EntityTexture;
	struct Fluid;
	struct Ore;
	struct RealmDetails;
	struct SoundPath;

	struct RegistryRegistry: NamedRegistry<Registry> {
		static Identifier ID() { return {"base", "registry/registry"}; }
		RegistryRegistry(): NamedRegistry(ID()) {}
	};

	struct ItemRegistry: NamedRegistry<Item> {
		static Identifier ID() { return {"base", "registry/item"}; }
		ItemRegistry(): NamedRegistry(ID()) {}
	};

	struct ItemTextureRegistry: NamedRegistry<ItemTexture> {
		static Identifier ID() { return {"base", "registry/item_texture"}; }
		ItemTextureRegistry(): NamedRegistry(ID()) {}
	};

	struct TextureRegistry: NamedRegistry<Texture> {
		static Identifier ID() { return {"base", "registry/texture"}; }
		TextureRegistry(): NamedRegistry(ID()) {}
	};

	struct EntityTextureRegistry: NamedRegistry<EntityTexture> {
		static Identifier ID() { return {"base", "registry/entity_texture"}; }
		EntityTextureRegistry(): NamedRegistry(ID()) {}
	};

	struct EntityFactoryRegistry: NamedRegistry<EntityFactory> {
		static Identifier ID() { return {"base", "registry/entity_factory"}; }
		EntityFactoryRegistry(): NamedRegistry(ID()) {}
	};

	struct TilesetRegistry: NamedRegistry<Tileset> {
		static Identifier ID() { return {"base", "registry/tileset"}; }
		TilesetRegistry(): NamedRegistry(ID()) {}
	};

	struct TileEntityFactoryRegistry: NamedRegistry<TileEntityFactory> {
		static Identifier ID() { return {"base", "registry/tile_entity_factory"}; }
		TileEntityFactoryRegistry(): NamedRegistry(ID()) {}
	};

	struct OreRegistry: NamedRegistry<Ore> {
		static Identifier ID() { return {"base", "registry/ore"}; }
		OreRegistry(): NamedRegistry(ID()) {}
	};

	struct RealmFactoryRegistry: NamedRegistry<RealmFactory> {
		static Identifier ID() { return {"base", "registry/realm_factory"}; }
		RealmFactoryRegistry(): NamedRegistry(ID()) {}
	};

	struct RealmTypeRegistry: IdentifierRegistry {
		static Identifier ID() { return {"base", "registry/realm_type"}; }
		RealmTypeRegistry(): IdentifierRegistry(ID()) {}
	};

	struct RealmDetailsRegistry: NamedRegistry<RealmDetails> {
		static Identifier ID() { return {"base", "registry/realm_details"}; }
		RealmDetailsRegistry(): NamedRegistry(ID()) {}
	};

	struct PacketFactoryRegistry: NumericRegistry<PacketFactory> {
		static Identifier ID() { return {"base", "registry/packet_factory"}; }
		PacketFactoryRegistry(): NumericRegistry(ID()) {}
	};

	struct LocalCommandFactoryRegistry: StringRegistry<LocalCommandFactory> {
		static Identifier ID() { return {"base", "registry/local_command_factory"}; }
		LocalCommandFactoryRegistry(): StringRegistry(ID()) {}
	};

	struct FluidRegistry: NamedRegistry<Fluid> {
		static Identifier ID() { return {"base", "registry/fluid"}; }
		FluidRegistry(): NamedRegistry(ID()) {}
	};

	struct TileRegistry: NamedRegistry<Tile> {
		static Identifier ID() { return {"base", "registry/tile"}; }
		TileRegistry(): NamedRegistry(ID()) {}
		void addMineable(Identifier, const ItemStack &, bool consumable);
		void addMineable(Identifier, ItemStack &&, bool consumable);
	};

	struct CropRegistry: NamedRegistry<Crop> {
		static Identifier ID() { return {"base", "registry/crop"}; }
		CropRegistry(): NamedRegistry(ID()) {}
	};

	struct ModuleFactoryRegistry: NamedRegistry<ModuleFactory> {
		static Identifier ID() { return {"base", "registry/module_factory"}; }
		ModuleFactoryRegistry(): NamedRegistry(ID()) {}
	};

	struct ItemSetRegistry: NamedRegistry<ItemSet> {
		static Identifier ID() { return {"base", "registry/itemset"}; }
		ItemSetRegistry(): NamedRegistry(ID()) {}
	};

	struct SoundRegistry: NamedRegistry<SoundPath> {
		static Identifier ID() { return {"base", "registry/sound"}; }
		SoundRegistry(): NamedRegistry(ID()) {}
	};

	struct ResourceRegistry: NamedRegistry<Resource> {
		static Identifier ID() { return {"base", "registry/resource"}; }
		ResourceRegistry(): NamedRegistry(ID()) {}
	};
}
