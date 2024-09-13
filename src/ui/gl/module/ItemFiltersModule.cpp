#include "ui/gl/module/ItemFiltersModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Button.h"
#include "ui/gl/widget/Checkbox.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/Constants.h"

namespace Game3 {
	ItemFiltersModule::ItemFiltersModule(UIContext &ui, const std::shared_ptr<ClientGame> &game, const std::any &argument):
		ItemFiltersModule(ui, game, std::any_cast<DirectedPlace>(argument)) {}

	ItemFiltersModule::ItemFiltersModule(UIContext &ui, const std::shared_ptr<ClientGame> &game, const DirectedPlace &place):
		Module(ui), weakGame(game), place(place) {}

	void ItemFiltersModule::init() {
		auto vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical);
		vbox->insertAtEnd(shared_from_this());

		auto first_hbox = std::make_shared<Box>(ui, scale, Orientation::Horizontal, 2, 0, Color{});
		first_hbox->insertAtEnd(vbox);

		auto copy_button = std::make_shared<Button>(ui, scale);
		copy_button->setText("Copy");
		copy_button->setVerticalAlignment(Alignment::Middle);
		copy_button->setOnClick([this](Widget &, int button, int, int) {
			if (button == 1)
				copy();
			return true;
		});
		copy_button->insertAtEnd(first_hbox);

		dropSlot = std::make_shared<ItemSlot>(ui, -1, INNER_SLOT_SIZE, SLOT_SCALE);
		dropSlot->setHorizontalExpand(true);
		dropSlot->insertAtEnd(first_hbox);

		auto paste_button = std::make_shared<Button>(ui, scale);
		paste_button->setText("Paste");
		paste_button->setVerticalAlignment(Alignment::Middle);
		paste_button->setOnClick([this](Widget &, int button, int, int) {
			if (button == 1)
				paste();
			return true;
		});
		paste_button->insertAtEnd(first_hbox);

		auto second_hbox = std::make_shared<Box>(ui, scale, Orientation::Horizontal, 2, 0, Color{});
		second_hbox->insertAtEnd(vbox);

		whitelistCheckbox = std::make_shared<Checkbox>(ui, scale);
		whitelistCheckbox->setFixedSize(8 * scale);
		whitelistCheckbox->insertAtEnd(second_hbox);

		auto whitelist_label = std::make_shared<Label>(ui, scale);
		whitelist_label->setText("Whitelist");
		whitelist_label->setHorizontalExpand(true);
		whitelist_label->insertAtEnd(second_hbox);

		strictCheckbox = std::make_shared<Checkbox>(ui, scale);
		strictCheckbox->setFixedSize(8 * scale);
		strictCheckbox->insertAtEnd(second_hbox);

		auto strict_label = std::make_shared<Label>(ui, scale);
		strict_label->setText("Strict");
		// strict_label->setHorizontalExpand(true);
		strict_label->insertAtEnd(second_hbox);
	}

	void ItemFiltersModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode ItemFiltersModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void ItemFiltersModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	void ItemFiltersModule::copy() {

	}

	void ItemFiltersModule::paste() {

	}
}
