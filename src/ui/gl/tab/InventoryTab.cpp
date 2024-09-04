#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/widget/Scroller.h"
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
		Tab(ui)  {}

	void InventoryTab::init() {
		assert(!playerInventoryModule);
		playerInventoryModule = makePlayerInventoryModule(ui);

		assert(!playerScroller);
		playerScroller = makePlayerScroller();

		assert(!moduleScroller);
		moduleScroller = makeModuleScroller();
	}

	void InventoryTab::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		Rectangle rectangle(x, y, width, height);

		std::unique_lock<DefaultMutex> module_lock;
		if (Module *active_module = getModule(module_lock)) {
			auto saver = renderers.getSaver();
			renderers.rectangle.drawOnScreen(Color{0.6, 0.3, 0, 0.2}, x + (rectangle.width - SEPARATOR_WIDTH) / 2, y, SEPARATOR_WIDTH, rectangle.height);
			rectangle.width = (rectangle.width - SEPARATION) / 2;
			renderers.updateSize(rectangle.width, rectangle.height);

			{
				auto saver = ui.scissorStack.pushRelative(Rectangle(rectangle.width + SEPARATION, 0) + rectangle, renderers);
				moduleScroller->render(ui, renderers, rectangle);
				module_lock.unlock();
			}

			auto subsaver = ui.scissorStack.pushRelative(rectangle, renderers);
			playerScroller->render(ui, renderers, rectangle);
		} else {
			playerScroller->render(ui, renderers, rectangle);
		}
	}

	void InventoryTab::renderIcon(const RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/inventory.png"));
	}

	bool InventoryTab::click(UIContext &ui, int button, int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->click(ui, button, x, y))
			return true;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				return moduleScroller->click(ui, button, x, y);

		return false;
	}

	bool InventoryTab::dragStart(UIContext &ui, int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->dragStart(ui, x, y))
			return true;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				return moduleScroller->dragStart(ui, x, y);

		return false;
	}

	bool InventoryTab::dragEnd(UIContext &ui, int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->dragEnd(ui, x, y))
			return true;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				return moduleScroller->dragEnd(ui, x, y);

		return false;
	}

	bool InventoryTab::scroll(UIContext &ui, float x_delta, float y_delta, int x, int y) {
		if (playerScroller->getLastRectangle().contains(x, y) && playerScroller->scroll(ui, x_delta, y_delta, x, y))
			return true;

		std::unique_lock<DefaultMutex> lock;
		if (getModule(lock))
			if (moduleScroller->getLastRectangle().contains(x, y))
				return moduleScroller->scroll(ui, x_delta, y_delta, x, y);

		return false;
	}

	void InventoryTab::setModule(std::shared_ptr<Module> new_module) {
		assert(new_module);
		auto lock = activeModule.uniqueLock();
		(activeModule.getBase() = std::move(new_module))->reset();
		moduleScroller->setChild(activeModule);
		activeModule->init(ui);
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

	std::shared_ptr<Scroller> InventoryTab::makePlayerScroller() {
		auto scroller = std::make_shared<Scroller>(scale);
		scroller->setChild(playerInventoryModule);
		scroller->insertAtEnd(shared_from_this());
		return scroller;
	}

	std::shared_ptr<Scroller> InventoryTab::makeModuleScroller() {
		auto scroller = std::make_shared<Scroller>(scale);
		scroller->setChild(activeModule.copyBase());
		scroller->insertAtEnd(shared_from_this());
		return scroller;
	}
}
