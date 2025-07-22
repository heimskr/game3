#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/dialog/ConnectionDialog.h"
#include "ui/widget/Aligner.h"
#include "ui/widget/Box.h"
#include "ui/widget/IconButton.h"
#include "ui/widget/Spinner.h"
#include "ui/TitleUI.h"
#include "ui/Window.h"
#include "util/Promises.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		constexpr std::array paths{
			"resources/gui/stone.png",
			"resources/gui/dirt.png",
			"resources/gui/grass.png",
			"resources/gui/grimrubble.png",
			"resources/tileset/lava/tile.png",
		};

		TexturePtr background = cacheTexture(choose(paths, std::random_device{}));
	}

	void TitleUI::init(Window &window) {
		Dialog::init();

		updater = Updater::make();

		aligner = make<Aligner>(ui, Orientation::Horizontal, Alignment::End);
		hbox = make<Box>(ui, 1, Orientation::Horizontal);

		updateButton = make<IconButton>(ui, 1);
		updateButton->setIconTexture(cacheTexture("resources/gui/up.png"));
		updateButton->setTooltipText("Download update");
		updateButton->setOnClick([this, &window](Widget &) {
			if (updating.exchange(true)) {
				window.error("Already updating.");
				return;
			}

			try {
				if (updater->mayUpdate() || 1) {
					setSpinning(true);
					updater->updateFetch()->then([&window, weak = getWeakSelf()](bool result) {
						if (auto self = weak.lock()) {
							self->setSpinning(false);
							if (result) {
								window.alert("Updated successfully.");
							} else {
								window.alert("Nothing to update.");
							}
							self->updating = false;
						}
					});
				} else {
					window.alert("The game chose not to update.");
					updating = false;
				}
			} catch (const std::exception &error) {
				setSpinning(false);
				window.error(std::format("Failed to update:\n{}", error.what()));
			}
		});

		spinner = make<Spinner>(ui, 1);
		spinner->setFixedSize(8, 8);

		updateButton->insertAtEnd(hbox);
		updateButton->setFixedHeight(10.f);
		hbox->insertAtEnd(aligner);
		aligner->insertAtEnd(shared_from_this());
		ui.emplaceDialog<ConnectionDialog>(1);
	}

	void TitleUI::render(const RendererContext &renderers) {
		Window &window = ui.window;

		constexpr float strength = 0.3;
		renderers.singleSprite.drawOnScreen(background, RenderOptions{
			.sizeX = static_cast<double>(window.getWidth()),
			.sizeY = static_cast<double>(window.getHeight()),
			.scaleX = 2 * window.uiContext.scale,
			.scaleY = 2 * window.uiContext.scale,
			.color{strength, strength, strength, 1},
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		static float hue = 0;
		static TexturePtr gangblanc = cacheTexture("resources/gangblanc.png");

		if (const auto mouse = window.getMouseCoordinates<double>()) {
			const auto [mouse_x, mouse_y] = *mouse;
			renderers.singleSprite.drawOnScreen(gangblanc, RenderOptions{
				.x = mouse_x,
				.y = mouse_y,
				.scaleX = 16,
				.scaleY = 16,
				.invertY = false,
			});
		}

		auto now = getTime();

		if (window.lastRenderTime) {
			hue += std::chrono::duration_cast<std::chrono::nanoseconds>(now - *window.lastRenderTime).count() / 1e9 * 144 * 0.001;
		}

		window.lastRenderTime = now;

		OKHsv hsv{hue, 1, 1, 1};
		constexpr int i_max = 32;
		constexpr double offset_factor = 2.05;
		constexpr double x_offset = offset_factor * i_max;

		for (int i = 0; i < i_max; ++i) {
			const double offset = offset_factor * (i_max - i);
			renderers.text.drawOnScreen("game3", TextRenderOptions{
				.x = window.getWidth() / 2.0 + (offset - x_offset) / 2,
				.y = 64.0 + offset / 2,
				.scaleX = 2.5,
				.scaleY = 2.5,
				.color = hsv.convert<Color>(),
				.align = TextAlign::Center,
				.alignTop = true,
			});

			hsv.hue += 0.5 / i_max;
		}

		firstChild->render(renderers, getPosition());
	}

	Rectangle TitleUI::getPosition() const {
		Rectangle position = ui.window.inset(10);
		position.height = getScale() * 32;
		return position;
	}

	std::shared_ptr<TitleUI> TitleUI::getSelf() {
		return std::static_pointer_cast<TitleUI>(shared_from_this());
	}

	std::weak_ptr<TitleUI> TitleUI::getWeakSelf() {
		return std::weak_ptr(getSelf());
	}

	void TitleUI::setSpinning(bool spinning) {
		if (spinning == (spinner->getParent() != nullptr)) {
			// No change
			return;
		}

		if (spinning) {
			hbox->remove(updateButton);
			spinner->insertAtEnd(hbox);
		} else {
			hbox->remove(spinner);
			updateButton->insertAtEnd(hbox);
		}
	}
}
