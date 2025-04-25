#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tileentity/Building.h"

namespace Game3 {
	Building::Building(Identifier id, Position position, RealmID innerRealmID, Position entrance, Identifier soundSetID):
		TileEntity(std::move(id), ID(), position, true),
		innerRealmID(innerRealmID),
		entrance(entrance),
		soundSetID(std::move(soundSetID)) {}

	void Building::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = ensureObject(json);
		object["innerRealmID"] = innerRealmID;
		object["entrance"] = boost::json::value_from(entrance);
	}

	bool Building::onInteractOn(const std::shared_ptr<Player> &player, Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) {
		return onInteractNextTo(player, modifiers, used_item, hand);
	}

	bool Building::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		if (getSide() == Side::Client) {
			return false;
		}

		if (const SoundSetPtr &sound_set = getSoundSet()) {
			float pitch = 1;
			if (float variance = sound_set->pitchVariance; variance != 1) {
				pitch = threadContext.getPitch(variance);
			}

			player->getRealm()->playSound(player->getPosition(), sound_set->choose(), pitch, 64);
		}

		teleport(player);
		return true;
	}

	void Building::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		innerRealmID = boost::json::value_to<RealmID>(json.at("innerRealmID"));
		entrance = boost::json::value_to<Position>(json.at("entrance"));
	}

	void Building::teleport(const std::shared_ptr<Entity> &entity) {
		entity->teleport(entrance, getInnerRealm());
	}

	std::shared_ptr<Realm> Building::getInnerRealm() const {
		GamePtr game = getGame();
		return game->getRealm(innerRealmID);
	}

	void Building::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << innerRealmID;
		buffer << entrance;
		if (soundSet) {
			buffer << soundSet->identifier;
		} else {
			buffer << soundSetID;
		}
	}

	void Building::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> innerRealmID;
		buffer >> entrance;
		buffer >> soundSetID;
		soundSet.reset();
	}

	const SoundSetPtr & Building::getSoundSet() {
		if (!soundSet && soundSetID) {
			soundSet = getGame()->registry<SoundSetRegistry>().at(soundSetID);
		}

		return soundSet;
	}
}
