#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/JumpPacket.h"
#include "ui/Canvas.h"
#include "ui/TextRenderer.h"

namespace Game3 {
	ClientPlayer::ClientPlayer(): Player() {}

	std::shared_ptr<ClientPlayer> ClientPlayer::create(Game &) {
		return Entity::create<ClientPlayer>();
	}

	void ClientPlayer::render(SpriteRenderer &sprites, TextRenderer &text) {
		Player::render(sprites, text);

		text.drawOnMap(displayName, {
			.x = static_cast<float>(position.column) + offset.x + .525f,
			.y = static_cast<float>(position.row) + offset.y - offset.z + .025f,
			.color = {0.f, 0.f, 0.f, 1.f},
			.align = TextAlign::Center,
		});

		text.drawOnMap(displayName, {
			.x = static_cast<float>(position.column) + offset.x + .5f,
			.y = static_cast<float>(position.row) + offset.y - offset.z,
			.color = {1.f, 1.f, 1.f, 1.f},
			.align = TextAlign::Center,
		});
	}

	void ClientPlayer::stopContinuousInteraction() {
		send(ContinuousInteractionPacket());
	}

	void ClientPlayer::setContinuousInteraction(bool on, Modifiers modifiers) {
		if (on != continuousInteraction) {
			continuousInteraction = on;
			if (on)
				send(ContinuousInteractionPacket(modifiers));
			else
				send(ContinuousInteractionPacket());
		}

		continuousInteractionModifiers = modifiers;
	}

	void ClientPlayer::jump() {
		if (abs(offset.z) <= 0.001f)
			send(JumpPacket());
	}

	const std::unordered_set<Layer> & ClientPlayer::getVisibleLayers() const {
		static std::unordered_set<Layer> main_layers     {Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest};
		static std::unordered_set<Layer> with_item_pipes {Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest, Layer::ItemPipes, Layer::ItemExtractors};

		if (const auto *stack = inventory->getActive()) {
			static const std::unordered_set<Identifier> item_set {"base:item/wrench"_id, "base:item/item_pipe"_id};
			if (item_set.contains(stack->item->identifier))
				return with_item_pipes;
		}

		return main_layers;
	}
}
