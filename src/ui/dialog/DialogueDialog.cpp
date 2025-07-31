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

		std::filesystem::path neutral = "resources/talksprites/rhosum_neutral.png";
		std::filesystem::path surprised = "resources/talksprites/rhosum_surprised.png";

		graph->addNode("entry", "You want it?", {{"Yeah!", "shop"}, {"What's up?", "whats_up"}, {"No.", "oh_ok"}}, neutral);
		graph->addNode("whats_up", "Just chillin.", {{"Wait a second", "entry"}, {"Ok.", "!exit"}}, surprised);
		graph->addNode("oh_ok", "Oh, ok.", {{"Ok.", "!exit"}}, surprised);
		graph->addNode("purchase_failed", "Sorry, I can't give credit. Come back when you're a little, mmm, richer!", {{"Hold up.", "shop"}, {"Sure.", "!exit"}}, neutral);
		graph->addNode("purchase_successful", "It's yours, my friend.", {{"More.", "shop"}, {"Ok bye.", "!exit"}});
		graph->addNode(std::make_shared<RhosumShopNode>(graph, "shop", DialogueOptions{{"", "!exit"}, {"", "no_credit"}, {"", "purchased"}}));

		ui.getPlayer()->dialogueGraph = graph;
	}

	void DialogueDialog::render(const RendererContext &renderers) {
		auto fail = [&] {
			ui.window.queue([](Window &window) {
				window.uiContext.removeDialogs<DialogueDialog>();
			});
		};

		if (!graph) {
			fail();
			return;
		}

		auto lock = graph->uniqueLock();

		if (!graph->getStillOpen()) {
			fail();
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
