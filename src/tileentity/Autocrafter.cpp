#include "game/EnergyContainer.h"
#include "game/Game.h"
#include "game/InventorySpan.h"
#include "game/ServerInventory.h"
#include "graphics/ItemTexture.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "item/Furniture.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/Autocrafter.h"
#include "ui/module/AutocrafterModule.h"

namespace Game3 {
	namespace {
		constexpr EnergyAmount ENERGY_CAPACITY = 100'000;
		constexpr EnergyAmount ENERGY_PER_ACTION = 200;
		constexpr std::chrono::milliseconds PERIOD{100};
		constexpr ItemCount INPUT_CAPACITY  = 10;
		constexpr ItemCount OUTPUT_CAPACITY = 10;
	}

	Autocrafter::Autocrafter():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Autocrafter::Autocrafter(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Autocrafter::Autocrafter(Position position_):
		TileEntity("base:tile/autocrafter_base", ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	const InventoryPtr & Autocrafter::getInventory(InventoryID index) const {
		if (index == 0)
			return HasInventory::getInventory(0);
		if (index == 1)
			return stationInventory;
		throw std::invalid_argument("Couldn't retrieve inventory with index " + std::to_string(index));
	}

	void Autocrafter::setInventory(InventoryPtr inventory, InventoryID index) {
		if (index == 0) {
			HasInventory::setInventory(std::move(inventory), 0);
		} else if (index == 1) {
			stationInventory = std::move(inventory);
			connectStationInventory();
		} else
			throw std::invalid_argument("Couldn't set inventory with index " + std::to_string(index));
	}

	bool Autocrafter::mayInsertItem(const ItemStack &, Direction, Slot slot) {
		return slot == Slot(-1) || slot < Slot(INPUT_CAPACITY);
	}

	bool Autocrafter::mayExtractItem(Direction, Slot slot) {
		return Slot(INPUT_CAPACITY) <= slot && slot < Slot(INPUT_CAPACITY + OUTPUT_CAPACITY);
	}

	EnergyAmount Autocrafter::getEnergyCapacity() {
		assert(energyContainer);
		return energyContainer->copyCapacity();
	}

	void Autocrafter::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY), 0);
		stationInventory = Inventory::create(shared_from_this(), 1, 1);
		connectStationInventory();
	}

	void Autocrafter::tick(Game &game, float delta) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, game, delta};

		autocraft();
		enqueueTick(PERIOD);
	}

	bool Autocrafter::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, ItemStack *, Hand) {
		if (getSide() == Side::Client)
			return false;

		if (modifiers.onlyAlt()) {
			{
				const InventoryPtr inventory = getInventory(0);
				auto lock = inventory->sharedLock();
				inventory->iterate([&](const ItemStack &stack, Slot) {
					player->give(stack);
					return false;
				});
			}
			{
				auto lock = stationInventory.uniqueLock();
				if (ItemStack *station_stack = (*stationInventory)[0])
					player->give(std::move(*station_stack));
			}
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack(realm->getGame(), "base:item/autocrafter"_id));
			return true;
		}

		if (modifiers.onlyShift()) {
			EnergeticTileEntity::addObserver(player, false);
		} else {
			player->send(OpenModuleForAgentPacket(AutocrafterModule::ID(), getGID()));
			InventoriedTileEntity::addObserver(player, true);
		}

		auto lock = energyContainer->sharedLock();
		return true;
	}

	void Autocrafter::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		if (cachedTile == TileID(-1) || tileLookupFailed) {
			if (tileID.empty()) {
				tileLookupFailed = true;
				cachedTile = 0;
				cachedUpperTile = 0;
			} else {
				tileLookupFailed = false;
				cachedTile = tileset["base:tile/autocrafter_base"_id];
				cachedUpperTile = tileset.getUpper(cachedTile);
				cachedArmLower = tileset["base:tile/autocrafter_arm"_id];
				cachedArmUpper = tileset.getUpper(cachedArmLower);
				if (cachedUpperTile == 0)
					cachedUpperTile = -1;
			}
		}

		if (cachedTile == 0)
			return;

		const auto tilesize = tileset.getTileSize();
		const auto texture = tileset.getTexture(realm->getGame());
		const auto base_x = (cachedTile % (texture->width / tilesize)) * tilesize;
		const auto base_y = (cachedTile / (texture->width / tilesize)) * tilesize;

		sprite_renderer(texture, {
			.x = float(position.column),
			.y = float(position.row),
			.offsetX = base_x / 2.f,
			.offsetY = base_y / 2.f,
			.sizeX = float(tilesize),
			.sizeY = float(tilesize),
		});
	}

	void Autocrafter::renderUpper(SpriteRenderer &sprite_renderer) {
		if (!isVisible({-1, 0}))
			return;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		if (cachedUpperTile == TileID(-1) || cachedUpperTile == 0)
			return;

		const auto tilesize = tileset.getTileSize();
		const auto texture = tileset.getTexture(realm->getGame());
		const auto base_x = (cachedUpperTile % (texture->width / tilesize)) * tilesize;
		const auto base_y = (cachedUpperTile / (texture->width / tilesize)) * tilesize;

		sprite_renderer(texture, {
			.x = float(position.column),
			.y = float(position.row - 1),
			.offsetX = base_x / 2.f,
			.offsetY = base_y / 2.f,
			.sizeX = float(tilesize),
			.sizeY = float(tilesize),
		});

		if (stationTexture) {
			sprite_renderer(stationTexture, {
				.x = position.column + .125f,
				.y = position.row    - .2f,
				.offsetX = stationXOffset,
				.offsetY = stationYOffset,
				.sizeX = stationSizeX,
				.sizeY = stationSizeY,
				.scaleX = .75f * 16.f / stationSizeX,
				.scaleY = .75f * 16.f / stationSizeY,
			});
		}

		const auto arm_upper_x = (cachedArmUpper % (texture->width / tilesize)) * tilesize;
		const auto arm_upper_y = (cachedArmUpper / (texture->width / tilesize)) * tilesize;

		sprite_renderer(texture, {
			.x = float(position.column),
			.y = float(position.row - 1),
			.offsetX = arm_upper_x / 2.f,
			.offsetY = arm_upper_y / 2.f,
			.sizeX = float(tilesize),
			.sizeY = float(tilesize),
		});

		const auto arm_lower_x = (cachedArmLower % (texture->width / tilesize)) * tilesize;
		const auto arm_lower_y = (cachedArmLower / (texture->width / tilesize)) * tilesize;

		sprite_renderer(texture, {
			.x = float(position.column),
			.y = float(position.row),
			.offsetX = arm_lower_x / 2.f,
			.offsetY = arm_lower_y / 2.f,
			.sizeX = float(tilesize),
			.sizeY = float(tilesize),
		});
	}

	void Autocrafter::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		auto station_lock = stationInventory.sharedLock();
		json["stationInventory"] = dynamic_cast<ServerInventory &>(*stationInventory);
	}

	void Autocrafter::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		if (auto iter = json.find("stationInventory"); iter != json.end()) {
			auto station_lock = stationInventory.sharedLock();
			stationInventory = std::make_shared<ServerInventory>(ServerInventory::fromJSON(game, *iter, shared_from_this()));
		}
	}

	void Autocrafter::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		HasInventory::encode(buffer, 0);
		HasInventory::encode(buffer, 1);
		EnergeticTileEntity::encode(game, buffer);
		buffer << target;
	}

	void Autocrafter::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		HasInventory::decode(buffer, 0);
		HasInventory::decode(buffer, 1);
		EnergeticTileEntity::decode(game, buffer);
		target = buffer.take<Identifier>();
		stationSet();
	}

	void Autocrafter::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const TileEntityPacket packet(getSelf());
		const SetTileEntityEnergyPacket energy_packet = makeEnergyPacket();

		{
			auto energetic_lock = EnergeticTileEntity::observers.uniqueLock();
			std::erase_if(EnergeticTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
				if (auto player = weak_player.lock()) {
					player->send(energy_packet);
					return false;
				}

				return true;
			});
		}

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();
		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});
	}

	void Autocrafter::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "SetTarget") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			bool success = false;

			Identifier new_target;

			try {
				new_target = buffer->take<Identifier>();
				success = setTarget(new_target);
			} catch (const std::invalid_argument &) {}

			if (source)
				sendMessage(source, "ModuleMessage", AutocrafterModule::ID(), "TargetSet", success, new_target);

		}
	}

	void Autocrafter::autocraft() {
		if (energyContainer->copyEnergy() < ENERGY_PER_ACTION)
			return;

		auto recipes_lock = cachedRecipes.sharedLock();
		if (cachedRecipes.empty())
			return;

		InventoryPtr inventory = getInventory(0);
		const ItemCount input_capacity = INPUT_CAPACITY;
		auto input_span = std::make_shared<InventorySpan>(inventory, 0, input_capacity - 1);
		auto output_span = std::make_shared<InventorySpan>(inventory, input_capacity, input_capacity + OUTPUT_CAPACITY - 1);
		Game &game = getGame();

		std::optional<std::vector<ItemStack>> leftovers;
		for (const std::shared_ptr<CraftingRecipe> &recipe: cachedRecipes) {
			if (recipe->craft(game, input_span, output_span, leftovers)) {
				auto energy_lock = energyContainer->sharedLock();
				energyContainer->remove(ENERGY_PER_ACTION, true);
				return;
			}
		}
	}

	bool Autocrafter::setTarget(Identifier new_target) {
		target = std::move(new_target);
		cacheRecipes();
		return true;
	}

	void Autocrafter::cacheRecipes() {
		auto lock = cachedRecipes.uniqueLock();
		cachedRecipes.clear();
		for (const std::shared_ptr<CraftingRecipe> &recipe: getGame().registry<CraftingRecipeRegistry>())
			if (validateRecipe(*recipe))
				cachedRecipes.push_back(recipe);
	}

	bool Autocrafter::stationSet() {
		auto station_lock = stationInventory.uniqueLock();

		bool out = true;

		if (ItemStack *stack = (*stationInventory)[0]) {
			if (auto station_item = std::dynamic_pointer_cast<StationFurniture>(stack->item)) {
				station = station_item->stationType;
				setStationTexture(*stack);
			} else {
				station = {};
				out = false;
				resetStationTexture();
			}
		} else {
			station = {};
			resetStationTexture();
		}

		cacheRecipes();
		return out;
	}

	void Autocrafter::setStationTexture(const ItemStack &stack) {
		Game &game = getGame();
		if (game.getSide() != Side::Client)
			return;
		std::shared_ptr<ItemTexture> item_texture = game.registry<ItemTextureRegistry>().at(stack.item->identifier);
		stationTexture = stack.getTexture(game);
		stationTexture->init();
		stationXOffset = item_texture->x / 2.f;
		stationYOffset = item_texture->y / 2.f;
		stationSizeX   = float(item_texture->width);
		stationSizeY   = float(item_texture->height);
	}

	void Autocrafter::resetStationTexture() {
		if (getSide() != Side::Client)
			return;
		stationTexture = {};
		stationXOffset = {};
		stationYOffset = {};
		stationSizeX   = {};
		stationSizeY   = {};
	}

	bool Autocrafter::validateRecipe(const CraftingRecipe &recipe) const {
		if (!recipe.stationType.empty() && recipe.stationType != station)
			return false;

		auto target_lock = target.sharedLock();

		for (const ItemStack &stack: recipe.output)
			if (stack.item->identifier == target)
				return true;

		return false;
	}

	void Autocrafter::connectStationInventory() {
		stationInventory->onMove = [this](Inventory &, Slot, Inventory &, Slot, bool) {
			return [this] {
				stationSet();
			};
		};

		stationInventory->onSwap = [this](Inventory &, Slot, Inventory &, Slot) {
			return [this] {
				stationSet();
			};
		};
	}
}
