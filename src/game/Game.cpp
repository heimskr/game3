#include "game/Game.h"
#include "util/Util.h"

namespace Game3 {
	void Game::initEntities() {
		for (const auto &[realm_id, realm]: realms)
			realm->initEntities();
	}

	void Game::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1'000'000'000.f;
		for (auto &[id, realm]: realms)
			realm->tick(delta);
	}

	RealmID Game::newRealmID() const {
		// TODO: a less stupid way of doing this.
		RealmID max = 1;
		for (const auto &[id, realm]: realms)
			max = std::max(max, id);
		return max + 1;
	}

	void to_json(nlohmann::json &json, const Game &game) {
		json["activeRealmID"] = game.activeRealm->id;
		json["realms"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[id, realm]: game.realms)
			json["realms"][std::to_string(id)] = nlohmann::json(*realm);
	}

	void from_json(const nlohmann::json &json, Game &game) {
		for (const auto &[string, realm_json]: json.at("realms").get<std::unordered_map<std::string, nlohmann::json>>())
			game.realms.emplace(parseUlong(string), std::make_shared<Realm>(realm_json)).first->second->game = &game;
		game.activeRealm = game.realms.at(json.at("activeRealmID"));
	}
}
