#include "game/Game.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"
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
		player->ticked = false;
	}

	RealmID Game::newRealmID() const {
		// TODO: a less stupid way of doing this.
		RealmID max = 1;
		for (const auto &[id, realm]: realms)
			max = std::max(max, id);
		return max + 1;
	}

	void Game::setText(const Glib::ustring &text, const Glib::ustring &name, bool focus, bool ephemeral) {
		if (canvas.window.textTab) {
			auto &tab = *canvas.window.textTab;
			tab.text = text;
			tab.name = name;
			tab.ephemeral = ephemeral;
			if (focus)
				tab.show();
			tab.reset(shared_from_this());
		}
	}

	const Glib::ustring & Game::getText() const {
		if (canvas.window.textTab)
			return canvas.window.textTab->text;
		throw std::runtime_error("Can't get text: TextTab is null");
	}

	void Game::click(int n, double pos_x, double pos_y) {
		if (!activeRealm)
			return;

		const auto &realm = *activeRealm;
		const auto &tilemap = realm.tilemap1;
		const auto scale = canvas.scale;

		pos_x -= canvas.width() / 2.f - (tilemap->width * tilemap->tileSize / 4.f) * scale + canvas.center.x() * canvas.magic * scale;
		pos_x /= tilemap->tileSize * scale / 2.f;

		pos_y -= canvas.height() / 2.f - (tilemap->height * tilemap->tileSize / 4.f) * scale + canvas.center.y() * canvas.magic * scale;
		pos_y /= tilemap->tileSize * scale / 2.f;

		const int x = pos_x;
		const int y = pos_y;

		(void) n;
		if (debugMode && player && 0 <= x && x < tilemap->width && 0 <= y && y < tilemap->height)
			player->teleport({y, x});
	}

	std::shared_ptr<Game> Game::create(Canvas &canvas) {
		return std::shared_ptr<Game>(new Game(canvas));
	}

	std::shared_ptr<Game> Game::fromJSON(const nlohmann::json &json, Canvas &canvas) {
		auto out = create(canvas);
		for (const auto &[string, realm_json]: json.at("realms").get<std::unordered_map<std::string, nlohmann::json>>())
			out->realms.emplace(parseUlong(string), Realm::fromJSON(realm_json)).first->second->setGame(*out);
		out->activeRealm = out->realms.at(json.at("activeRealmID"));
		if (json.contains("debugMode"))
			out->debugMode = json.at("debugMode");
		else
			out->debugMode = false;
		return out;
	}

	void to_json(nlohmann::json &json, const Game &game) {
		json["activeRealmID"] = game.activeRealm->id;
		json["debugMode"] = game.debugMode;
		json["realms"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[id, realm]: game.realms)
			json["realms"][std::to_string(id)] = nlohmann::json(*realm);
	}
}
