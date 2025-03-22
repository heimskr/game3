#include "data/ConsumptionRule.h"
#include "data/ProductionRule.h"
#include "game/Game.h"
#include "recipe/BiomassLiquefierRecipe.h"
#include "recipe/CentrifugeRecipe.h"
#include "recipe/CombinerRecipe.h"
#include "recipe/CraftingRecipe.h"
#include "recipe/DissolverRecipe.h"
#include "recipe/GeothermalRecipe.h"
#include "recipe/LiquefierRecipe.h"
#include "registry/Registries.h"

namespace Game3 {
	void Game::initRegistries() {
		registries.clear();
		registries.add<CraftingRecipeRegistry>();
		itemRegistry = registries.add<ItemRegistry>();
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
		tileRegistry = registries.add<TileRegistry>();
		registries.add<CropRegistry>();
		registries.add<CentrifugeRecipeRegistry>();
		registries.add<GeothermalRecipeRegistry>();
		registries.add<ModuleFactoryRegistry>();
		registries.add<ItemSetRegistry>();
		registries.add<DissolverRecipeRegistry>();
		registries.add<CombinerRecipeRegistry>();
		registries.add<SoundRegistry>();
		registries.add<ResourceRegistry>();
		registries.add<ProductionRuleRegistry>();
		registries.add<ConsumptionRuleRegistry>();
		registries.add<LiquefierRecipeRegistry>();
		registries.add<BiomassLiquefierRecipeRegistry>();
		registries.add<AttributeExemplarRegistry>();
		registries.add<MinigameFactoryRegistry>();
		registries.add<StatusEffectFactoryRegistry>();
	}
}
