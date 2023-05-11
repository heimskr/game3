#include "ThreadContext.h"
#include "Tileset.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/LocalClient.h"
#include "packet/CommandPacket.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"

namespace Game3 {
	void ClientGame::click(int button, int, double pos_x, double pos_y) {
		if (!activeRealm)
			return;

		auto &realm = *activeRealm;
		const auto width  = canvas.width();
		const auto height = canvas.height();

		// Lovingly chosen by trial and error.
		if (0 < realm.ghostCount && width - 40.f <= pos_x && pos_x < width - 16.f && height - 40.f <= pos_y && pos_y < height - 16.f) {
			realm.confirmGhosts();
			return;
		}

		const auto [x, y] = translateCanvasCoordinates(pos_x, pos_y);

		if (button == 1) {
			if (auto *stack = player->inventory->getActive())
				stack->item->use(player->inventory->activeSlot, *stack, {{y, x}, activeRealm, player}, {});
		} else if (button == 3 && player && !realm.rightClick({y, x}, pos_x, pos_y) && debugMode) {
			player->teleport({y, x});
		}
	}

	Gdk::Rectangle ClientGame::getVisibleRealmBounds() const {
		const auto [left,     top] = translateCanvasCoordinates(0., 0.);
		const auto [right, bottom] = translateCanvasCoordinates(canvas.width(), canvas.height());
		return {
			static_cast<int>(left),
			static_cast<int>(top),
			static_cast<int>(right - left + 1),
			static_cast<int>(bottom - top + 1),
		};
	}

	MainWindow & ClientGame::getWindow() {
		return canvas.window;
	}

	Position ClientGame::translateCanvasCoordinates(double x, double y) const {
		auto &realm = *activeRealm;
		const auto scale = canvas.scale;
		const auto tile_size  = realm.getTileset().getTileSize();
		constexpr auto map_length = CHUNK_SIZE * REALM_DIAMETER;
		x -= canvas.width() / 2.f - (map_length * tile_size / 4.f) * scale + canvas.center.x() * canvas.magic * scale;
		y -= canvas.height() / 2.f - (map_length * tile_size / 4.f) * scale + canvas.center.y() * canvas.magic * scale;
		const double sub_x = x < 0.? 1. : 0.;
		const double sub_y = y < 0.? 1. : 0.;
		x /= tile_size * scale / 2.f;
		y /= tile_size * scale / 2.f;
		return {static_cast<Index>(x - sub_x), static_cast<Index>(y - sub_y)};
	}

	void ClientGame::activateContext() {
		canvas.window.activateContext();
	}

	void ClientGame::setText(const Glib::ustring &text, const Glib::ustring &name, bool focus, bool ephemeral) {
		if (canvas.window.textTab) {
			auto &tab = *canvas.window.textTab;
			tab.text = text;
			tab.name = name;
			tab.ephemeral = ephemeral;
			if (focus)
				tab.show();
			tab.reset(toClientPointer());
		}
	}

	const Glib::ustring & ClientGame::getText() const {
		if (canvas.window.textTab)
			return canvas.window.textTab->text;
		throw std::runtime_error("Can't get text: TextTab is null");
	}

	void ClientGame::runCommand(const std::string &command) {
		CommandPacket packet(threadContext.rng(), command);
		client->send(packet);
	}

	void ClientGame::tick() {

	}
}
