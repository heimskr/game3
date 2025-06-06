#include "data/RegisterableIdentifier.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "threading/ThreadContext.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/ContextMenu.h"
#include "ui/gl/widget/CraftingSlider.h"
#include "ui/gl/widget/Grid.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/ItemSlot.h"
#include "ui/gl/widget/ProgressBar.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/widget/Spacer.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

namespace Game3 {
	void CraftingTab::init() {
		inventoryModule = ui.makePlayerInventoryModule();
		// Prevent shift clicking trying to move from the inventory to itself.
		inventoryModule->setOnSlotClick([](Slot, Modifiers) { return true; });
		hbox = make<Box>(ui, selfScale, Orientation::Horizontal, 0, 1);
		recipeList = make<Box>(ui, selfScale, Orientation::Vertical);

		hbox->setHorizontalExpand(true);

		auto inventory_scroller = make<Scroller>(ui, selfScale);
		inventory_scroller->setChild(inventoryModule);
		inventory_scroller->setExpand(true, true);
		hbox->append(inventory_scroller);

		auto recipe_scroller = make<Scroller>(ui, selfScale);
		recipe_scroller->setChild(recipeList);
		recipe_scroller->setExpand(true, true);
		hbox->append(recipe_scroller);

		hbox->insertAtEnd(shared_from_this());
	}

	void CraftingTab::render(const RendererContext &renderers, float x, float y, float width, float height) {
		maybeRemeasure(renderers, width, height);
		assert(firstChild != nullptr);
		Tab::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	void CraftingTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/crafting.png"));
	}

	void CraftingTab::onFocus() {
		if (resetQueued) {
			resetQueued = false;
			reset();
		}
	}

	void CraftingTab::reset() {
		recipeList->clearChildren();

		ClientPlayerPtr player = ui.getPlayer();
		if (!player) {
			return;
		}

		auto known = player->craftingManager.getKnownRecipes();

		for (const std::vector<CraftingRecipePtr> *set: {&known.full, &known.partial}) {
			for (const CraftingRecipePtr &recipe: *set) {
				recipeList->append(make<RecipeRow>(ui, selfScale, recipe));
			}
		}
	}

	void CraftingTab::queueReset() {
		if (isActive()) {
			reset();
		} else {
			resetQueued = true;
		}
	}

	RecipeRow::RecipeRow(UIContext &ui, float scale, CraftingRecipePtr recipe):
		Box(ui, scale, Orientation::Horizontal, 0),
		recipe(std::move(recipe)) {}

	void RecipeRow::init() {
		auto grid = make<Grid>(ui, selfScale);
		grid->setRowSpacing(1);
		grid->setColumnSpacing(1);
		grid->attach(make<Label>(ui, selfScale, "In:"), 0, 0);
		grid->attach(make<Label>(ui, selfScale, "Out:"), 1, 0);

		GamePtr game = ui.getGame();
		auto &exemplars = game->registry<AttributeExemplarRegistry>();
		auto &items = *game->itemRegistry;

		std::size_t column = 1;

		const std::vector<CraftingRequirement> inputs = recipe->getInput(game);
		constexpr int icon_scale = 8;

		for (const CraftingRequirement &requirement: inputs) {
			if (requirement.is<ItemStackPtr>()) {
				auto item_slot = make<ItemSlot>(ui, -1, INNER_SLOT_SIZE, selfScale / 2);
				item_slot->setStack(requirement.get<ItemStackPtr>());
				grid->attach(std::move(item_slot), 0, column);
			} else {
				const AttributeRequirement &attribute_requirement = requirement.get<AttributeRequirement>();
				const Identifier &attribute = attribute_requirement.attribute;
				std::shared_ptr<RegisterableIdentifier> item_id = exemplars.maybe(attribute);
				if (item_id == nullptr) {
					auto icon = make<Icon>(ui, selfScale);
					icon->setFixedSize(icon_scale * selfScale, icon_scale * selfScale);
					icon->setIconTexture(cacheTexture("resources/gui/question_mark.png"));
					grid->attach(std::move(icon), 0, column);
				} else {
					auto item_slot = make<ItemSlot>(ui, -1, INNER_SLOT_SIZE, selfScale / 2);
					item_slot->setStack(ItemStack::create(game, items.at(item_id->get()), attribute_requirement.count));
					item_slot->setTooltipText(std::format("Any {}", attribute.getPostPath()));
					grid->attach(std::move(item_slot), 0, column);
				}
			}

			++column;
		}

		column = 1;

		for (const ItemStackPtr &output: recipe->getOutput(inputs, game)) {
			auto item_slot = make<ItemSlot>(ui, -1, INNER_SLOT_SIZE, selfScale / 2);
			item_slot->setStack(output);
			grid->attach(std::move(item_slot), 1, column++);
		}

		if (recipe->stationType) {
			auto vbox = make<Box>(ui, selfScale, Orientation::Vertical, 0);
			vbox->append(grid);
			vbox->append(make<Label>(ui, selfScale * 0.75, std::format("Station: {}", recipe->stationType.getPostPath())));
			append(std::move(vbox));
		} else {
			append(grid);
		}

		append(make<Spacer>(ui, Orientation::Horizontal));
		append(make<CraftingSlider>(ui, selfScale * 0.75, recipe));
	}
}
