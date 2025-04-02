#include "entity/Player.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "interface/HasFluidType.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tileentity/EternalFountain.h"
#include "ui/gl/module/MultiModule.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
	}

	EternalFountain::EternalFountain() = default;

	EternalFountain::EternalFountain(Identifier tileID, Position position):
		TileEntity(std::move(tileID), ID(), position, true) {}

	EternalFountain::EternalFountain(Position position):
		EternalFountain("base:tile/eternal_fountain"_id, position) {}

	size_t EternalFountain::getMaxFluidTypes() const {
		return 100;
	}

	FluidAmount EternalFountain::getMaxLevel(FluidID) {
		return 1'000 * FluidTile::FULL;
	}

	void EternalFountain::init(Game &game) {
		HasFluids::init(safeDynamicCast<HasFluids>(shared_from_this()));
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 1), 0);
	}

	void EternalFountain::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server) {
			return;
		}

		Ticker ticker{*this, args};
		enqueueTick(PERIOD);

		const InventoryPtr inventory = getInventory(0);

		ItemStackPtr stack = inventory->withShared([](Inventory &inventory) {
			return (inventory)[0];
		});

		if (!stack) {
			return;
		}

		Identifier fluid_type;

		if (auto has_fluid_type = std::dynamic_pointer_cast<HasFluidType>(stack->item)) {
			fluid_type = has_fluid_type->getFluidType();
		} else {
			return;
		}

		if (fluid_type.empty()) {
			return;
		}

		auto fluids_lock = fluidContainer->levels.uniqueLock();
		const auto fluid_id = args.game->getFluid(fluid_type)->registryID;

		FluidAmount max = getMaxLevel(fluid_id);
		FluidAmount &level = fluidContainer->levels[fluid_id];
		if (level != max) {
			level = max;
			broadcast(false);
		}
	}

	void EternalFountain::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool EternalFountain::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/eternal_fountain"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Item, Substance::Fluid>::ID(), getGID()));
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return false;
	}

	void EternalFountain::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	void EternalFountain::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void EternalFountain::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void EternalFountain::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const auto packet = make<TileEntityPacket>(getSelf());

		auto fluid_holding_lock = FluidHoldingTileEntity::observers.uniqueLock();

		std::erase_if(FluidHoldingTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();

		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!FluidHoldingTileEntity::observers.contains(player)) {
					player->send(packet);
				}
				return false;
			}

			return true;
		});
	}

	GamePtr EternalFountain::getGame() const {
		return TileEntity::getGame();
	}
}
