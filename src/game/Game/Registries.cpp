#include "game/Game.h"
#include "recipe/CentrifugeRecipe.h"
#include "recipe/CraftingRecipe.h"
#include "recipe/GeothermalRecipe.h"
#include "registry/Registries.h"

namespace Game3 {
	void Game::initRegistries() {
		registries.clear();
		registries.add<CraftingRecipeRegistry>();
		registries.add<ItemRegistry>();
		registries.add<ItemTextureRegistry>();
		registries.add<TextureRegistry>();
		registries.add<EntityTextureRegistry>();
		registries.add<EntityFactoryRegistry>();
		registries.add<TilesetRegistry>();
		registries.add<TileEntityFactoryRegistry>();
		registries.add<OreRegistry>();
		registries.add<RealmFactoryRegistry>();
		registries.add<RealmTypeRegistry>();
		registries.add<RealmDetailsRegistry>();
		registries.add<PacketFactoryRegistry>();
		registries.add<LocalCommandFactoryRegistry>();
		registries.add<FluidRegistry>();
		registries.add<TileRegistry>();
		registries.add<CropRegistry>();
		registries.add<CentrifugeRecipeRegistry>();
		registries.add<GeothermalRecipeRegistry>();
		registries.add<ModuleFactoryRegistry>();
		registries.add<ItemSetRegistry>();
	}
}
