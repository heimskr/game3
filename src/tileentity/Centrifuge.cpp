#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "recipe/CentrifugeRecipe.h"
#include "tileentity/Centrifuge.h"
#include "ui/gl/module/MultiModule.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
	}

	Centrifuge::Centrifuge(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Centrifuge::Centrifuge(Position position_):
		Centrifuge("base:tile/centrifuge"_id, position_) {}

	size_t Centrifuge::getMaxFluidTypes() const {
		return 1;
	}

	FluidAmount Centrifuge::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Centrifuge::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 10), 0);
	}

	void Centrifuge::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};
		const GamePtr &game = args.game;

		enqueueTick(PERIOD);

		auto &levels = fluidContainer->levels;
		auto fluids_lock = levels.uniqueLock();

		if (levels.empty())
			return;

		auto &registry = game->registry<CentrifugeRecipeRegistry>();

		std::optional<ItemStackPtr> leftovers;
		const InventoryPtr inventory = getInventory(0);
		auto inventory_lock = inventory->uniqueLock();
		for (const std::shared_ptr<CentrifugeRecipe> &recipe: registry.items)
			if (recipe->craft(game, fluidContainer, inventory, leftovers))
				return;
	}

	void Centrifuge::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool Centrifuge::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/centrifuge"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Item, Substance::Fluid>::ID(), getGID()));
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		GamePtr game = realm->getGame();
		auto lock = fluidContainer->levels.sharedLock();
		for (const auto &[id, amount]: fluidContainer->levels)
			INFO("{} = {}", game->getFluid(id)->identifier, amount);
		return true;
	}

	void Centrifuge::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	void Centrifuge::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void Centrifuge::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void Centrifuge::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const auto packet = make<TileEntityPacket>(getSelf());

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();

		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});

		auto fluid_holding_lock = FluidHoldingTileEntity::observers.uniqueLock();

		std::erase_if(FluidHoldingTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!InventoriedTileEntity::observers.contains(player))
					player->send(packet);
				return false;
			}

			return true;
		});
	}

	GamePtr Centrifuge::getGame() const {
		return TileEntity::getGame();
	}
}
