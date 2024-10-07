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
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	void CraftingTab::init() {
		auto tab = shared_from_this();
		auto scroller = make<Scroller>(ui, scale);
		scroller->insertAtEnd(tab);

		inventoryModule = ui.makePlayerInventoryModule();
		hbox = make<Box>(ui, scale, Orientation::Horizontal, 0);
		recipeList = make<Box>(ui, scale, Orientation::Vertical);
		rightPane = make<Box>(ui, scale, Orientation::Vertical);

		inventoryModule->setHorizontalExpand(true);
		recipeList->setHorizontalExpand(true);
		rightPane->setHorizontalExpand(true);
		hbox->setHorizontalExpand(true);

		hbox->append(inventoryModule);
		hbox->append(recipeList);
		hbox->append(rightPane);

		scroller->setChild(hbox);
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
		SPAM("CraftingTab::reset()");

		ClientPlayerPtr player = ui.getPlayer();
		if (!player) {
			return;
		}

		auto known = player->craftingManager.getKnownRecipes();

		for (const std::vector<CraftingRecipePtr> *set: {&known.full, &known.partial}) {
			for (const CraftingRecipePtr &recipe: *set) {
				recipeList->append(make<RecipeRow>(*this, recipe));
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

	RecipeRow::RecipeRow(CraftingTab &parent, CraftingRecipePtr recipe):
		Grid(parent.getUI(), parent.getScale()),
		parent(parent),
		recipe(std::move(recipe)) {}

	void RecipeRow::init() {
		attach(make<Label>(ui, scale, "In:"), 0, 0);
		attach(make<Label>(ui, scale, "Out:"), 1, 0);

		GamePtr game = parent.getUI().getGame();
		auto &exemplars = game->registry<AttributeExemplarRegistry>();
		auto &items = game->registry<ItemRegistry>();

		std::size_t column = 1;

		for (const CraftingRequirement &requirement: recipe->getInput(game)) {
			auto icon = make<Icon>(ui, scale);
			icon->setFixedSize(8 * scale, 8 * scale);
			if (requirement.is<ItemStackPtr>()) {
				const ItemStackPtr &stack = requirement.get<ItemStackPtr>();
				TexturePtr texture = stack->getTexture();
				icon->setIconTexture(texture);
			} else {
				const Identifier &attribute = requirement.get<AttributeRequirement>().attribute;
				std::shared_ptr<RegisterableIdentifier> item_id = exemplars.maybe(attribute);
				if (item_id == nullptr) {
					icon->setIconTexture(cacheTexture("resources/gui/question_mark.png"));
				} else {
					auto stack = ItemStack::create(game, items.at(item_id->get()));
					icon->setIconTexture(stack->getTexture(*game));
				}
			}
			attach(icon, 0, column++);
		}

		column = 1;
	}
}
