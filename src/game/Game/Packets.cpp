#include "game/Game.h"
#include "packet/AddKnownItemPacket.h"
#include "packet/AgentMessagePacket.h"
#include "packet/ChatMessageSentPacket.h"
#include "packet/ChunkRequestPacket.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/ClickPacket.h"
#include "packet/CommandPacket.h"
#include "packet/CommandResultPacket.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/CraftPacket.h"
#include "packet/DestroyEntityPacket.h"
#include "packet/DestroyTileEntityPacket.h"
#include "packet/DisplayTextPacket.h"
#include "packet/DoVillageTradePacket.h"
#include "packet/DragPacket.h"
#include "packet/DropItemPacket.h"
#include "packet/EntityChangingRealmsPacket.h"
#include "packet/EntityMoneyChangedPacket.h"
#include "packet/EntityMovedPacket.h"
#include "packet/EntityPacket.h"
#include "packet/EntityRequestPacket.h"
#include "packet/EntityRiddenPacket.h"
#include "packet/EntitySetPathPacket.h"
#include "packet/ErrorPacket.h"
#include "packet/ExplosionPacket.h"
#include "packet/FluidUpdatePacket.h"
#include "packet/HeldItemSetPacket.h"
#include "packet/InteractPacket.h"
#include "packet/InventoryPacket.h"
#include "packet/InventorySlotUpdatePacket.h"
#include "packet/JumpPacket.h"
#include "packet/KnownItemsPacket.h"
#include "packet/LivingEntityHealthChangedPacket.h"
#include "packet/LoginPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/MovePlayerPacket.h"
#include "packet/MoveSlotsPacket.h"
#include "packet/OpenItemFiltersPacket.h"
#include "packet/OpenMinigamePacket.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/OpenVillageTradePacket.h"
#include "packet/PacketFactory.h"
#include "packet/PlaySoundPacket.h"
#include "packet/ProtocolVersionPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/RecipeListPacket.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/SelfTeleportedPacket.h"
#include "packet/SendChatMessagePacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/SetCopierConfigurationPacket.h"
#include "packet/SetFiringPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "packet/SetItemFiltersPacket.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "packet/StatusEffectsPacket.h"
#include "packet/SubmitScorePacket.h"
#include "packet/SubscribeToVillageUpdatesPacket.h"
#include "packet/SwapSlotsPacket.h"
#include "packet/TeleportSelfPacket.h"
#include "packet/TileEntityPacket.h"
#include "packet/TileEntityRequestPacket.h"
#include "packet/TileUpdatePacket.h"
#include "packet/TilesetRequestPacket.h"
#include "packet/TilesetResponsePacket.h"
#include "packet/TimePacket.h"
#include "packet/UpdateAgentFieldPacket.h"
#include "packet/UseFluidGunPacket.h"
#include "packet/UseItemPacket.h"
#include "packet/VillageUpdatePacket.h"

namespace Game3 {
	void Game::addPacketFactories() {
		auto &reg = registry<PacketFactoryRegistry>();

		auto add = [&reg](auto &&factory) {
			auto shared = std::make_shared<PacketFactory>(std::forward<decltype(factory)>(factory));
			reg.add(shared->number, shared);
		};

		add(PacketFactory::create<ProtocolVersionPacket>());
		add(PacketFactory::create<TileEntityPacket>());
		add(PacketFactory::create<ChunkRequestPacket>());
		add(PacketFactory::create<TileUpdatePacket>());
		add(PacketFactory::create<CommandResultPacket>());
		add(PacketFactory::create<CommandPacket>());
		add(PacketFactory::create<SelfTeleportedPacket>());
		add(PacketFactory::create<ChunkTilesPacket>());
		add(PacketFactory::create<RealmNoticePacket>());
		add(PacketFactory::create<LoginPacket>());
		add(PacketFactory::create<LoginStatusPacket>());
		add(PacketFactory::create<RegisterPlayerPacket>());
		add(PacketFactory::create<RegistrationStatusPacket>());
		add(PacketFactory::create<EntityPacket>());
		add(PacketFactory::create<MovePlayerPacket>());
		add(PacketFactory::create<ErrorPacket>());
		add(PacketFactory::create<EntityMovedPacket>());
		add(PacketFactory::create<SendChatMessagePacket>());
		add(PacketFactory::create<EntitySetPathPacket>());
		add(PacketFactory::create<TeleportSelfPacket>());
		add(PacketFactory::create<InteractPacket>());
		add(PacketFactory::create<InventorySlotUpdatePacket>());
		add(PacketFactory::create<DestroyEntityPacket>());
		add(PacketFactory::create<InventoryPacket>());
		add(PacketFactory::create<SetActiveSlotPacket>());
		add(PacketFactory::create<OpenItemFiltersPacket>());
		add(PacketFactory::create<DestroyTileEntityPacket>());
		add(PacketFactory::create<ClickPacket>());
		add(PacketFactory::create<TimePacket>());
		add(PacketFactory::create<CraftPacket>());
		add(PacketFactory::create<ContinuousInteractionPacket>());
		add(PacketFactory::create<FluidUpdatePacket>());
		add(PacketFactory::create<HeldItemSetPacket>());
		add(PacketFactory::create<SetHeldItemPacket>());
		add(PacketFactory::create<EntityRequestPacket>());
		add(PacketFactory::create<TileEntityRequestPacket>());
		add(PacketFactory::create<JumpPacket>());
		add(PacketFactory::create<DropItemPacket>());
		add(PacketFactory::create<OpenModuleForAgentPacket>());
		add(PacketFactory::create<SwapSlotsPacket>());
		add(PacketFactory::create<MoveSlotsPacket>());
		add(PacketFactory::create<AgentMessagePacket>());
		add(PacketFactory::create<SetTileEntityEnergyPacket>());
		add(PacketFactory::create<SetPlayerStationTypesPacket>());
		add(PacketFactory::create<EntityChangingRealmsPacket>());
		add(PacketFactory::create<ChatMessageSentPacket>());
		add(PacketFactory::create<DisplayTextPacket>());
		add(PacketFactory::create<DragPacket>());
		add(PacketFactory::create<SetItemFiltersPacket>());
		add(PacketFactory::create<TilesetRequestPacket>());
		add(PacketFactory::create<TilesetResponsePacket>());
		add(PacketFactory::create<RecipeListPacket>());
		add(PacketFactory::create<LivingEntityHealthChangedPacket>());
		add(PacketFactory::create<UseItemPacket>());
		add(PacketFactory::create<SubscribeToVillageUpdatesPacket>());
		add(PacketFactory::create<VillageUpdatePacket>());
		add(PacketFactory::create<OpenVillageTradePacket>());
		add(PacketFactory::create<DoVillageTradePacket>());
		add(PacketFactory::create<EntityMoneyChangedPacket>());
		add(PacketFactory::create<EntityRiddenPacket>());
		add(PacketFactory::create<SetCopierConfigurationPacket>());
		add(PacketFactory::create<PlaySoundPacket>());
		add(PacketFactory::create<KnownItemsPacket>());
		add(PacketFactory::create<AddKnownItemPacket>());
		add(PacketFactory::create<SubmitScorePacket>());
		add(PacketFactory::create<OpenMinigamePacket>());
		add(PacketFactory::create<SetFiringPacket>());
		add(PacketFactory::create<UseFluidGunPacket>());
		add(PacketFactory::create<StatusEffectsPacket>());
		add(PacketFactory::create<UpdateAgentFieldPacket>());
		add(PacketFactory::create<ExplosionPacket>());
	}
}
