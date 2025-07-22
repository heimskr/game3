#include "ui/widget/DialogueDisplay.h"
#include "ui/widget/Label.h"

namespace Game3 {
	DialogueDisplay::DialogueDisplay(UIContext &ui, float selfScale):
		Box(ui, selfScale, Orientation::Vertical, 2, 0.5) {}

	void DialogueDisplay::init() {
		WidgetPtr self = getSelf();

		mainText = make<Label>(ui, selfScale, "Hej.");
		mainText->insertAtEnd(self);

		optionBox = make<Box>(ui, selfScale, Orientation::Vertical, 0, 0);
		optionBox->insertAtEnd(self);

		optionBox->append(make<Label>(ui, selfScale, "> foo"));
		optionBox->append(make<Label>(ui, selfScale, "- bar"));
	}
}
