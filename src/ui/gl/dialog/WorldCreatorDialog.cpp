#include "graphics/Texture.h"
#include "ui/gl/dialog/WorldCreatorDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Format.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		constexpr float WIDTH = 100;
		constexpr float HEIGHT = 50;
	}

	WorldCreatorDialog::WorldCreatorDialog(UIContext &ui, float selfScale):
		DraggableDialog(ui, selfScale, WIDTH, HEIGHT) {
			setTitle("New World");
		}

	void WorldCreatorDialog::init() {
		DraggableDialog::init();

		auto grid = make<Grid>(ui, 1);
		grid->setRowSpacing(5);

		auto make_label = [&](UString text) {
			auto label = make<Label>(ui, selfScale, std::move(text));
			label->setVerticalAlignment(Alignment::Center);
			return label;
		};

		grid->attach(make_label("Path"), 0, 0);
		grid->attach(make_label("Seed"), 1, 0);

		pathInput = make<TextInput>(ui, selfScale);
		pathInput->setHorizontalExpand(true);
		pathInput->onSubmit.connect([this](TextInput &, const UString &) { submit(); });
		pathInput->setText("worlds/");
		grid->attach(pathInput, 0, 1);

		seedInput = make<TextInput>(ui, selfScale);
		seedInput->setHorizontalExpand(true);
		seedInput->onSubmit.connect([this](TextInput &, const UString &) { submit(); });
		grid->attach(seedInput, 1, 1);

		auto vbox = make<Box>(ui, 1, Orientation::Vertical, 0);

		vbox->append(std::move(grid));
		vbox->insertAtEnd(getSelf());

		recenter();
		ui.focusWidget(pathInput);
	}

	void WorldCreatorDialog::rescale(float new_scale) {
		position.width = dialogWidth * new_scale;
		position.height = dialogHeight * new_scale;
		DraggableDialog::rescale(new_scale);
	}

	void WorldCreatorDialog::submit() {
		auto self = getSelf();
		ui.removeDialog(self);

		std::optional<size_t> seed;

		if (const std::string &seed_text = seedInput->getText().raw(); !seed_text.empty()) {
			try {
				seed = parseNumber<size_t>(seed_text);
			} catch (const std::invalid_argument &) {
				seed = std::hash<std::string>{}(seed_text);
			}
		}

		signalSubmit(pathInput->getText().raw(), seed);
	}
}
