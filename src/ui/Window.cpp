#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/RendererContext.h"
#include "graphics/Tileset.h"
#include "types/Position.h"
#include "ui/gl/module/FluidsModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/ModuleFactory.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/OmniDialog.h"
#include "ui/Window.h"
#include "ui/Modifiers.h"
#include "ui/Window.h"

#include <fstream>

namespace Game3 {
	Window::Window(GLFWwindow &glfw_window):
		glfwWindow(&glfw_window),
		scale(8) {}

	void Window::queue(std::function<void(Window &)> function) {
		functionQueue.push(std::move(function));
	}

	int Window::getWidth() const {
		return 0;
	}

	int Window::getHeight() const {
		return 0;
	}

	int Window::getFactor() const {
		return 1;
	}

	int Window::getMouseX() const {
		return 0;
	}

	int Window::getMouseY() const {
		return 0;
	}

	std::pair<double, double> Window::getMouseCoordinates() const {
		return {static_cast<double>(getMouseX()), static_cast<double>(getMouseY())};
	}

	const std::shared_ptr<OmniDialog> & Window::getOmniDialog() {
		if (!omniDialog)
			omniDialog = std::make_shared<OmniDialog>(uiContext);
		return omniDialog;
	}

	void Window::showOmniDialog() {
		if (!uiContext.hasDialog<OmniDialog>())
			uiContext.addDialog(getOmniDialog());
	}

	void Window::closeOmniDialog() {
		uiContext.removeDialogs<OmniDialog>();
	}

	void Window::openModule(const Identifier &module_id, const std::any &argument) {
		auto &registry = game->registry<ModuleFactoryRegistry>();

		if (auto factory = registry[module_id]) {
			getOmniDialog();
			omniDialog->inventoryTab->setModule((*factory)(game, argument));
			omniDialog->activeTab = omniDialog->inventoryTab;
			if (!uiContext.hasDialog<OmniDialog>())
				uiContext.addDialog(omniDialog);
			return;
		}

		WARN("Couldn't find module {}", module_id);
	}

	void Window::removeModule() {
		getOmniDialog()->inventoryTab->removeModule();
	}

	void Window::alert(const UString &message, bool queue, bool modal, bool use_markup) {

	}

	void Window::error(const UString &message, bool queue, bool modal, bool use_markup) {

	}

	Modifiers Window::getModifiers() const {
		return {};
	}

	Position Window::getHoveredPosition() const {
		return {};
	}

	void Window::moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&buffer) {
		std::unique_lock<DefaultMutex> module_lock;

		if (Module *module_ = getOmniDialog()->inventoryTab->getModule(module_lock); module_ != nullptr && (module_id.empty() || module_->getID() == module_id)) {
			std::any data{std::move(buffer)};
			module_->handleMessage(source, name, data);
		}
	}

	void Window::activateContext() {
		glfwMakeContextCurrent(glfwWindow);
	}

	void Window::saveSettings() {
		std::ofstream ofs("settings.json");
		nlohmann::json json;
		{
			auto lock = settings.sharedLock();
			json = settings;
		}
		ofs << json.dump();
	}

	void Window::showExternalInventory(const std::shared_ptr<ClientInventory> &inventory) {
		assert(inventory);
		getOmniDialog()->inventoryTab->setModule(std::make_shared<InventoryModule>(uiContext, inventory));
	}

	void Window::showFluids(const std::shared_ptr<HasFluids> &has_fluids) {
		assert(has_fluids);
		getOmniDialog()->inventoryTab->setModule(std::make_shared<FluidsModule>(uiContext, has_fluids));
	}

	GlobalID Window::getExternalGID() const {
		if (omniDialog) {
			std::unique_lock<DefaultMutex> lock;
			if (Module *module_ = omniDialog->inventoryTab->getModule(lock)) {
				std::any empty;
				if (std::optional<Buffer> response = module_->handleMessage({}, "GetAgentGID", empty))
					return response->take<GlobalID>();
			}
		}

		return -1;
	}

	bool Window::inBounds(const Position &pos) const {
		const auto x = realmBounds.x;
		const auto y = realmBounds.y;
		return x <= pos.column && pos.column < x + realmBounds.width
		    && y <= pos.row    && pos.row    < y + realmBounds.height;
	}

	RendererContext Window::getRendererContext() {
		return {rectangleRenderer, singleSpriteRenderer, batchSpriteRenderer, textRenderer, circleRenderer, recolor, settings, getFactor()};
	}

	void Window::drawGL() {
		if (!game)
			return;

		const int factor = getFactor();
		int width  = getWidth()  * factor;
		int height = getHeight() * factor;

		game->activateContext();
		batchSpriteRenderer.update(*this);
		singleSpriteRenderer.update(*this);
		recolor.update(*this);
		textRenderer.update(*this);
		rectangleRenderer.update(width, height);
		textRenderer.update(width, height);
		circleRenderer.update(width, height);
		multiplier.update(width, height);
		overlayer.update(width, height);

		game->iterateRealms([](const RealmPtr &realm) {
			if (!realm->renderersReady)
				return;

			if (realm->wakeupPending.exchange(false)) {
				for (auto &row: *realm->baseRenderers)
					for (auto &renderer: row)
						renderer.wakeUp();

				for (auto &row: *realm->upperRenderers)
					for (auto &renderer: row)
						renderer.wakeUp();

				realm->reupload();
			} else if (realm->snoozePending.exchange(false)) {
				for (auto &row: *realm->baseRenderers)
					for (auto &renderer: row)
						renderer.snooze();

				for (auto &row: *realm->upperRenderers)
					for (auto &renderer: row)
						renderer.snooze();
			}
		});

		GLsizei tile_size = 16;
		RealmPtr realm = game->getActiveRealm();

		if (realm)
			tile_size = static_cast<GLsizei>(realm->getTileset().getTileSize());

		const auto static_size = static_cast<GLsizei>(REALM_DIAMETER * CHUNK_SIZE * tile_size * factor);

		if (mainTexture.getWidth() != width || mainTexture.getHeight() != height) {
			mainTexture.initRGBA(width, height, GL_NEAREST);
			staticLightingTexture.initRGBA(static_size, static_size, GL_NEAREST);
			dynamicLightingTexture.initRGBA(width, height, GL_NEAREST);
			scratchTexture.initRGBA(width, height, GL_NEAREST);

			GL::FBOBinder binder = fbo.getBinder();
			dynamicLightingTexture.useInFB();
			GL::clear(1, 1, 1);

			if (realm) {
				realm->queueStaticLightingTexture();
			}
		}

		bool do_lighting{};
		{
			auto lock = settings.sharedLock();
			do_lighting = settings.renderLighting;
		}

		if (!realm)
			return;

		if (do_lighting) {
			GL::FBOBinder binder = fbo.getBinder();
			mainTexture.useInFB();
			glViewport(0, 0, width, height); CHECKGL
			GL::clear(.2, .2, .2);
			RendererContext context = getRendererContext();
			context.updateSize(width, height);

			if (realm->prerender()) {
				mainTexture.useInFB();
				batchSpriteRenderer.update(*this);
				singleSpriteRenderer.update(*this);
				recolor.update(*this);
				textRenderer.update(*this);
				context.updateSize(getWidth(), getHeight());
				glViewport(0, 0, width, height); CHECKGL
				// Skip a frame to avoid glitchiness
				return;
			}

			realm->render(width, height, center, scale, context, game->getDivisor()); CHECKGL

			dynamicLightingTexture.useInFB();

			realm->renderLighting(width, height, center, scale, context, game->getDivisor()); CHECKGL

			scratchTexture.useInFB();
			GL::clear(1, 1, 1);

			singleSpriteRenderer.drawOnScreen(dynamicLightingTexture, RenderOptions{
				.x = 0,
				.y = 0,
				.sizeX = -1,
				.sizeY = -1,
				.scaleX = 1,
				.scaleY = 1,
			});

			ChunkPosition chunk = game->getPlayer()->getChunk() - ChunkPosition(1, 1);
			const auto [static_y, static_x] = chunk.topLeft();
			singleSpriteRenderer.drawOnMap(staticLightingTexture, RenderOptions{
				.x = double(static_x),
				.y = double(static_y),
				.sizeX = -1,
				.sizeY = -1,
				.scaleX = 1. / factor,
				.scaleY = 1. / factor,
				.viewportX = -double(factor),
				.viewportY = -double(factor),
			});

			binder.undo();

			context.updateSize(width, height);
			glViewport(0, 0, width, height); CHECKGL
			multiplier(mainTexture, scratchTexture);
		} else {
			RendererContext context = getRendererContext();
			glViewport(0, 0, width, height); CHECKGL
			GL::clear(.2, .2, .2);
			context.updateSize(width, height);

			if (realm->prerender()) {
				batchSpriteRenderer.update(*this);
				singleSpriteRenderer.update(*this);
				recolor.update(*this);
				textRenderer.update(*this);
				context.updateSize(width, height);
			}

			realm->render(width, height, center, scale, context, 1.f); CHECKGL
		}

		realmBounds = game->getVisibleRealmBounds();

		uiContext.render(getMouseX(), getMouseY());
	}
}
