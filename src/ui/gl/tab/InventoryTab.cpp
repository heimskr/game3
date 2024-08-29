#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/widget/ScrollerWidget.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Canvas.h"
#include "util/Defer.h"

namespace {
	constexpr int SEPARATION = 32;
	constexpr int SEPARATOR_WIDTH = 8;
}

namespace Game3 {
	namespace {
		std::shared_ptr<InventoryModule> makePlayerInventoryModule(UIContext &ui) {
			if (ClientPlayerPtr player = ui.getPlayer())
				return std::make_shared<InventoryModule>(ui.getGame(), std::static_pointer_cast<ClientInventory>(player->getInventory(0)));

			return std::make_shared<InventoryModule>(ui.getGame(), std::shared_ptr<ClientInventory>{});
		}
	}

	InventoryTab::InventoryTab(UIContext &ui):
		Tab(ui),
		playerInventoryModule(makePlayerInventoryModule(ui)),
		playerScroller(makePlayerScroller()),
		moduleScroller(makeModuleScroller()) {}

	void InventoryTab::render(UIContext &ui, const RendererContext &renderers) {
		Rectangle rectangle = ui.scissorStack.getTop();
		rectangle.x = 0;
		rectangle.y = 0;

		std::unique_lock<DefaultMutex> module_lock;
		if (Module *active_module = getModule(module_lock)) {
			auto saver = renderers.getSaver();
			renderers.rectangle.drawOnScreen(Color{0.6, 0.3, 0, 0.2}, (rectangle.width - SEPARATOR_WIDTH) / 2, 0, SEPARATOR_WIDTH, rectangle.height);
			rectangle.width = (rectangle.width - SEPARATION) / 2;
			renderers.updateSize(rectangle.width, rectangle.height);

			{
				ui.scissorStack.pushRelative(Rectangle(rectangle.width + SEPARATION, 0) + rectangle, true);
				Defer pop([&] { ui.scissorStack.pop(); });
				moduleScroller->render(ui, renderers, rectangle);
				module_lock.unlock();
			}

			ui.scissorStack.pushRelative(rectangle, true);
			Defer pop([&] { ui.scissorStack.pop(); });
			playerScroller->render(ui, renderers, rectangle);
		} else {
			playerScroller->render(ui, renderers, rectangle);
		}
	}

	void InventoryTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/inventory.png"));
	}

	void InventoryTab::click(int button, int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->click(ui, button, x, y))
			return;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				moduleScroller->click(ui, button, x, y);
	}

	void InventoryTab::dragStart(int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->dragStart(ui, x, y))
			return;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				moduleScroller->dragStart(ui, x, y);
	}

	void InventoryTab::dragEnd(int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->dragEnd(ui, x, y))
			return;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				moduleScroller->dragEnd(ui, x, y);
	}

	void InventoryTab::scroll(float x_delta, float y_delta, int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->scroll(ui, x_delta, y_delta, x, y))
			return;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				moduleScroller->scroll(ui, x_delta, y_delta, x, y);
	}

	void InventoryTab::setModule(std::shared_ptr<Module> new_module) {
		assert(new_module);
		auto lock = activeModule.uniqueLock();
		(activeModule.getBase() = std::move(new_module))->reset();
		moduleScroller->setChild(activeModule);
	}

	Module * InventoryTab::getModule(std::shared_lock<DefaultMutex> &lock) {
		if (activeModule)
			lock = activeModule.sharedLock();
		return activeModule.getBase().get();
	}

	Module * InventoryTab::getModule(std::unique_lock<DefaultMutex> &lock) {
		if (activeModule)
			lock = activeModule.uniqueLock();
		return activeModule.getBase().get();
	}

	void InventoryTab::removeModule() {
		moduleScroller->setChild(nullptr);
		activeModule.reset();
	}

	std::shared_ptr<ScrollerWidget> InventoryTab::makePlayerScroller() {
		auto scroller = std::make_shared<ScrollerWidget>();
		scroller->setChild(playerInventoryModule);
		return scroller;
	}

	std::shared_ptr<ScrollerWidget> InventoryTab::makeModuleScroller() {
		auto scroller = std::make_shared<ScrollerWidget>();
		scroller->setChild(activeModule.copyBase());
		return scroller;
	}
}
