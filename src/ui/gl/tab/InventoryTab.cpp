#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

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
		playerInventoryModule(makePlayerInventoryModule(ui)) {}

	void InventoryTab::render(UIContext &ui, RendererContext &renderers) {
		Rectangle rectangle = ui.scissorStack.getTop();

		std::unique_lock<DefaultMutex> module_lock;
		if (Module *active_module = getModule(module_lock)) {
			rectangle.width /= 2;
			rectangle.x += rectangle.width;
			active_module->render(ui, renderers, rectangle);
			rectangle.x -= rectangle.width;
			module_lock.unlock();
		}

		playerInventoryModule->render(ui, renderers, rectangle);
	}

	void InventoryTab::renderIcon(RendererContext &renderers) {
		renderIconTexture(renderers, cacheTexture("resources/gui/inventory.png"));
	}

	void InventoryTab::click(int x, int y) {
		if (playerInventoryModule->getLastRectangle().contains(x, y) && playerInventoryModule->click(ui, x, y))
			return;

		std::unique_lock<DefaultMutex> lock;
		if (Module *active_module = getModule(lock))
			if (active_module->getLastRectangle().contains(x, y))
				active_module->click(ui, x, y);
	}

	void InventoryTab::dragStart(int x, int y) {
		if (playerInventoryModule->getLastRectangle().contains(x, y) && playerInventoryModule->dragStart(ui, x, y))
			return;

		std::unique_lock<DefaultMutex> lock;
		if (Module *active_module = getModule(lock))
			if (active_module->getLastRectangle().contains(x, y))
				active_module->dragStart(ui, x, y);

	}

	void InventoryTab::dragEnd(int x, int y) {
		if (playerInventoryModule->getLastRectangle().contains(x, y) && playerInventoryModule->dragEnd(ui, x, y))
			return;

		std::unique_lock<DefaultMutex> lock;
		if (Module *active_module = getModule(lock))
			if (active_module->getLastRectangle().contains(x, y))
				active_module->dragEnd(ui, x, y);
	}

	void InventoryTab::setModule(std::shared_ptr<Module> new_module) {
		assert(new_module);
		auto lock = activeModule.uniqueLock();
		(activeModule.getBase() = std::move(new_module))->reset();
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
		activeModule.reset();
	}
}
