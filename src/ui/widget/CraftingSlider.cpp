#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "graphics/Texture.h"
#include "packet/CraftPacket.h"
#include "recipe/CraftingRecipe.h"
#include "ui/widget/Box.h"
#include "ui/widget/Button.h"
#include "ui/widget/CraftingSlider.h"
#include "ui/widget/Icon.h"
#include "ui/widget/IconButton.h"
#include "ui/widget/IntegerInput.h"
#include "ui/widget/Slider.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "util/Util.h"

namespace Game3 {
	CraftingSlider::CraftingSlider(UIContext &ui, float scale, CraftingRecipePtr recipe):
		Grid(ui, scale),
		recipe(std::move(recipe)) {}

	void CraftingSlider::init() {
		clearChildren();

		auto make_increment = [this](ssize_t delta) {
			return [this, delta](Widget &) {
				increment(delta);
			};
		};

		auto minus = make<Icon>(ui, selfScale);
		minus->setIconTexture(cacheTexture("resources/gui/minus.png"));
		minus->setFixedSize(8 * selfScale);
		minus->setOnClick(make_increment(-1));
		attach(std::move(minus), 0, 0);

		valueSlider = make<Slider>(ui, selfScale);
		valueSlider->setRange(1.0, static_cast<double>(getMaximum()));
		valueSlider->setStep(1.0);
		valueSlider->setDisplayDigits(0);
		valueSlider->setValue(1.0);
		valueSlider->onValueUpdate.connect([this](Slider &, double new_value) {
			setValue(static_cast<std::size_t>(new_value));
		});
		attach(valueSlider, 0, 1);

		auto plus = make<Icon>(ui, selfScale);
		plus->setIconTexture(cacheTexture("resources/gui/plus.png"));
		plus->setFixedSize(8 * selfScale);
		plus->setOnClick(make_increment(1));
		attach(std::move(plus), 0, 2);

		auto hbox = make<Box>(ui, selfScale, Orientation::Horizontal, selfScale / 2, 0, Color{});

		auto one_button = make<Button>(ui, selfScale);
		one_button->setText("1");
		one_button->setOnClick([this](Widget &) {
			setValue(1);
		});
		hbox->append(std::move(one_button));

		valueInput = make<IntegerInput>(ui, selfScale);
		valueInput->setFixedWidth(16 * selfScale);
		valueInput->setText(std::format("{}", value));
		valueInput->onChange.connect([this](TextInput &input, const UString &text) {
			if (text.empty()) {
				value = 1;
				return;
			}

			try {
				value = std::min(getMaximum(), parseNumber<std::size_t>(text.raw()));
			} catch (const std::invalid_argument &) {
				input.setText(std::to_string(value));
			}
		});
		hbox->append(valueInput);

		auto craft_button = make<IconButton>(ui, selfScale);
		craft_button->setIconTexture(cacheTexture("resources/gui/crafting.png"));
		craft_button->setOnClick([this](Widget &) {
			craft();
		});
		hbox->append(std::move(craft_button));

		auto max_button = make<Button>(ui, selfScale);
		max_button->setText(std::format("{}", getMaximum()));
		max_button->setOnClick([this](Widget &) {
			setValue(getMaximum());
		});
		hbox->append(std::move(max_button));

		attach(std::move(hbox), 1, 1);

		Grid::init();
	}

	void CraftingSlider::setValue(std::size_t new_value) {
		value = new_value;
		valueInput->setText(std::to_string(value));
		valueSlider->setValue(static_cast<double>(value));
	}

	void CraftingSlider::increment(ssize_t delta) {
		setValue(std::min(getMaximum(), static_cast<std::size_t>(std::max(1z, static_cast<ssize_t>(value) + delta))));
	}

	void CraftingSlider::craft() {
		static size_t nextID = 1;
		ui.getPlayer()->send(make<CraftPacket>(nextID++, recipe->registryID, value));
	}

	std::size_t CraftingSlider::getMaximum() const {
		return 64;
	}
}
