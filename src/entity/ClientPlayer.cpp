#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/LocalClient.h"
#include "packet/AgentMessagePacket.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/JumpPacket.h"
#include "packet/MovePlayerPacket.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/TextRenderer.h"

namespace Game3 {
	ClientPlayer::ClientPlayer(): Player() {}

	std::shared_ptr<ClientPlayer> ClientPlayer::create(Game &) {
		return Entity::create<ClientPlayer>();
	}

	void ClientPlayer::render(SpriteRenderer &sprites, TextRenderer &text) {
		Player::render(sprites, text);

		const auto [column, row] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		text.drawOnMap(displayName, {
			.x = static_cast<float>(column) + x + .525f,
			.y = static_cast<float>(row) + y - z + .025f,
			.color = {0.f, 0.f, 0.f, 1.f},
			.align = TextAlign::Center,
		});

		text.drawOnMap(displayName, {
			.x = static_cast<float>(column) + x + .5f,
			.y = static_cast<float>(row) + y - z,
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
		static std::unordered_set<Layer> main_layers {Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest};
		return main_layers;
	}

	bool ClientPlayer::move(Direction direction, MovementContext context) {
		const bool moved = Entity::move(direction, context);
		if (moved)
			send(MovePlayerPacket(position, direction, context.facingDirection, offset));
		return moved;
	}

	void ClientPlayer::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "ModuleMessage") {
			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const auto module_name = buffer->take<Identifier>();
			const auto message_name = buffer->take<std::string>();
			getGame().toClient().getWindow().moduleMessageBuffer(module_name, source, message_name, std::move(*buffer));
		}
	}

	void ClientPlayer::sendMessage(const std::shared_ptr<Agent> &destination, const std::string &name, std::any &data) {
		assert(destination);
		if (auto *buffer = std::any_cast<Buffer>(&data))
			getGame().toClient().client->send(AgentMessagePacket(destination->getGID(), name, *buffer));
		else
			throw std::runtime_error("Expected data to be a Buffer in ClientPlayer::sendMessage");
	}
}
