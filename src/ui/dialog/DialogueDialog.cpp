#include "dialogue/RhosumShopNode.h"
#include "entity/ClientPlayer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"
#include "graphics/Texture.h"
#include "types/UString.h"
#include "ui/dialog/DialogueDialog.h"
#include "ui/widget/DialogueDisplay.h"
#include "ui/widget/Scroller.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"

namespace Game3 {
	DialogueDialog::DialogueDialog(UIContext &ui, float selfScale):
		Dialog(ui, selfScale) {}

	void DialogueDialog::init() {
		dialogueGraph = std::make_shared<DialogueGraph>(ui.getPlayer());

		dialogueGraph->addNode("entry", "Hey there.", {{"SHOPPE.", "shop"}, {"What's up?", "whats_up"}, {"No.", "oh_ok"}}, "resources/talksprites/rhosum_neutral.png");
		dialogueGraph->addNode("whats_up", "Just chillin.", {{"Wait a second", "entry"}, {"Ok.", "!exit"}}, "resources/talksprites/rhosum_surprised.png");
		dialogueGraph->addNode("oh_ok", "Oh, ok.", {{"Ok.", "!exit"}}, "resources/talksprites/rhosum_surprised.png");
		dialogueGraph->addNode(std::make_shared<RhosumShopNode>(*dialogueGraph, "shop"));

		dialogueDisplay = make<DialogueDisplay>(ui, selfScale, dialogueGraph);
		dialogueScroller = make<Scroller>(ui, selfScale);
		dialogueScroller->setChild(dialogueDisplay);
		dialogueScroller->insertAtEnd(getSelf());
	}

	void DialogueDialog::render(const RendererContext &renderers) {
		if (!dialogueDisplay->getStillOpen()) {
			ui.window.queue([](Window &window) {
				window.uiContext.removeDialogs<DialogueDialog>();
			});
			return;
		}

		if (dialogueDisplay->getGraph()->getActiveNode()->render(ui, renderers)) {
			return;
		}

		Rectangle position = getPosition();

		{
			auto saver = ui.scissorStack.pushAbsolute(position, renderers);
			ui.drawFrame(renderers, getScale(), false, FRAME_PIECES, DEFAULT_BACKGROUND_COLOR);
		}

		Rectangle scroller_position = position.shrinkAll(7 * getScale());
		dialogueScroller->maybeRemeasure(renderers, scroller_position.width, scroller_position.height);
		dialogueScroller->render(renderers, scroller_position);

		if (TexturePtr face_texture = dialogueDisplay->getFaceTexture()) {
			float face_scale = getScale();
			position.x += position.width - face_scale * face_texture->width;
			position.y -= face_scale * face_texture->height;
			renderers.singleSprite.drawOnScreen(face_texture, RenderOptions::simple(position.x, position.y, face_scale, -1.0 / face_scale, -1.0 / face_scale));
		}
	}

	Rectangle DialogueDialog::getPosition() const {
		int ui_width = ui.getWidth();
		int ui_height = ui.getHeight();

		constexpr float x_leftover = 0.2;
		constexpr int height = 500;
		constexpr int y_gap = 32;

		return Rectangle(ui_width * x_leftover, ui_height - height - y_gap, ui_width * (1.0 - 2 * x_leftover), height);
	}

	bool DialogueDialog::hidesHotbar() const {
		return true;
	}

	bool DialogueDialog::keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat) {
		if (dialogueGraph->getActiveNode()->keyPressed(key, modifiers, is_repeat)) {
			return true;
		}

		return dialogueDisplay->keyPressed(key, modifiers, is_repeat);
	}
}
