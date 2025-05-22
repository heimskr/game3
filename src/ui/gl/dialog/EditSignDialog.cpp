#include "graphics/Texture.h"
#include "ui/gl/dialog/EditSignDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/IconButton.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Log.h"

namespace Game3 {
	EditSignDialog::EditSignDialog(UIContext &ui, float selfScale, int width, int height, UString initialTilename, UString initialContents):
		DraggableDialog(ui, selfScale, width, height),
		initialTilename(std::move(initialTilename)),
		initialContents(std::move(initialContents)) {}

	void EditSignDialog::render(const RendererContext &renderers) {
		DraggableDialog::render(renderers);
		vbox->render(renderers, bodyRectangle);
	}

	void EditSignDialog::init() {
		DraggableDialog::init();

		vbox = make<Box>(ui, selfScale, Orientation::Vertical, 2, 0, Color{});
		vbox->insertAtEnd(shared_from_this());

		grid = make<Grid>(ui, selfScale);
		grid->setRowSpacing(2.5);
		grid->insertAtEnd(vbox);

		for (int i = 0; const char *string: {"Tilename", "Contents"}) {
			auto label = make<Label>(ui, selfScale, string);
			label->setVerticalAlignment(Alignment::Center);
			grid->attach(std::move(label), i++, 0);
		}

		tilenameInput = make<TextInput>(ui, selfScale);
		tilenameInput->setText(std::move(initialTilename));
		tilenameInput->setHorizontalExpand(true);
		grid->attach(tilenameInput, 0, 1);

		contentsInput = make<TextInput>(ui, selfScale);
		contentsInput->setText(std::move(initialContents));
		contentsInput->setHorizontalExpand(true);
		contentsInput->setMultiline(true);
		contentsInput->setFixedHeight(-1);
		grid->attach(contentsInput, 1, 1);

		buttonBox = make<Box>(ui, selfScale, Orientation::Horizontal, 2, 0, Color{});
		vbox->append(buttonBox);

		auto spacer = make<Label>(ui, selfScale);
		spacer->setHorizontalExpand(true);
		buttonBox->append(std::move(spacer));

		auto okay_button = make<Button>(ui, selfScale);
		okay_button->setText("Okay");
		okay_button->setOnClick([this](Widget &) {
			DialogPtr self = getSelf();
			onSubmit(tilenameInput->getText(), contentsInput->getText());
			ui.removeDialog(self);
		});
		buttonBox->append(std::move(okay_button));
	}

	bool EditSignDialog::keyPressed(uint32_t key, Modifiers modifiers, bool) {
		if (key == GLFW_KEY_ESCAPE && modifiers.empty()) {
			ui.removeDialog(getSelf());
			return true;
		}

		return false;
	}

	void EditSignDialog::childResized(const WidgetPtr &, Orientation, int new_width, int new_height) {
		const auto scale = getScale();

		if (new_width >= 0) {
			position.width = new_width + 4 * scale;
		}

		if (new_height >= 0) {
			position.height = new_height + 12 * scale;
		}
	}
}
