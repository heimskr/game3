#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "packet/AddKnownItemPacket.h"
#include "ui/gl/dialog/OmniDialog.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	AddKnownItemPacket::AddKnownItemPacket(Identifier item_id):
		itemID(std::move(item_id)) {}

	void AddKnownItemPacket::handle(const ClientGamePtr &game) {
		if (game->getPlayer()->addKnownItem(itemID)) {
			game->getWindow()->queue([](Window &window) {
				window.getOmniDialog()->craftingTab->reset();
			});
		}
	}
}
