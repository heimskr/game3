#include "dialogue/RhosumShopNode.h"
#include "game/ClientGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "item/Item.h"
#include "ui/widget/ItemSlot.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	RhosumShopNode::RhosumShopNode(DialogueGraph &parent, std::string name, TexturePtr faceOverride):
		DialogueNode(parent, std::move(name), {}, std::move(faceOverride)) {
			if (this->faceOverride == nullptr) {
				this->faceOverride = cacheTexture("resources/talksprites/rhosum_surprised.png");
			}

			ClientGamePtr game = parent.getGame();
			UIContext &ui = game->getUIContext();
			lampOil = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/lamp_oil"), -1, INNER_SLOT_SIZE, 1);
			rope = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/rope"), -1, INNER_SLOT_SIZE, 1);
			bomb = make<ItemSlot>(ui, nullptr, ItemStack::create(game, "base:item/bomb"), -1, INNER_SLOT_SIZE, 1);

			lampOil->setOnClick([](Widget &) {
				INFO("lamp oil!");
			});
		}

	bool RhosumShopNode::render(UIContext &ui, const RendererContext &renderers) {
		assert(faceOverride != nullptr);

		int window_width = ui.window.getWidth();
		int window_height = ui.window.getHeight();
		float width = 74 * ui.scale;
		float height = 34 * ui.scale;
		int face_width = faceOverride->width;
		int face_height = faceOverride->height;

		Rectangle position((window_width - width) / 2, (window_height - height + face_height * ui.scale) / 2, width, height);

		{
			auto saver = ui.scissorStack.pushAbsolute(position, renderers);
			ui.drawFrame(renderers, ui.scale, false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);

			lampOil->render(renderers, Rectangle(ui.scale * 9, ui.scale * 9, -1, -1));
			rope->render(renderers, Rectangle(ui.scale * 29, ui.scale * 9, -1, -1));
			bomb->render(renderers, Rectangle(ui.scale * 49, ui.scale * 9, -1, -1));
		}

		position.x = (window_width - face_width * ui.scale) / 2;
		position.y -= face_height * ui.scale;

		renderers.singleSprite.drawOnScreen(faceOverride, RenderOptions::simple(position.x, position.y, ui.scale));

		return true;
	}
}
