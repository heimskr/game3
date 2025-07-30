#include "dialogue/RhosumShop.h"
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
		graph = DialogueGraph::create(ui.getPlayer());

		graph->addNode("entry", "Hey there.", {{"SHOPPE.", "shop"}, {"What's up?", "whats_up"}, {"No.", "oh_ok"}}, "resources/talksprites/rhosum_neutral.png");
		graph->addNode("whats_up", "Just chillin.", {{"Wait a second", "entry"}, {"Ok.", "!exit"}}, "resources/talksprites/rhosum_surprised.png");
		graph->addNode("oh_ok", "Oh, ok.", {{"Ok.", "!exit"}}, "resources/talksprites/rhosum_surprised.png");
		graph->addNode(std::make_shared<RhosumShopNode>(graph, "shop"));
	}

	void DialogueDialog::render(const RendererContext &renderers) {
		if (!graph || !graph->getStillOpen()) {
			ui.window.queue([](Window &window) {
				window.uiContext.removeDialogs<DialogueDialog>();
			});
			return;
		}

		if (auto current_node = graph->getActiveNode(); current_node != lastNode) {
			lastNode = std::move(current_node);
			setDialogueWidget(lastNode->getWidget(ui));
		}

		dialogueWidget->render(renderers, ui.scissorStack.getBase());
	}

	Rectangle DialogueDialog::getPosition() const {
		return ui.scissorStack.getBase();
	}

	bool DialogueDialog::hidesHotbar() const {
		return true;
	}

	void DialogueDialog::setDialogueWidget(WidgetPtr widget) {
		if (dialogueWidget) {
			remove(dialogueWidget);
		}

		widget->insertAtEnd(getSelf());
		dialogueWidget = std::move(widget);
	}
}
