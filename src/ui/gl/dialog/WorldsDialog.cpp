#include "graphics/Texture.h"
#include "ui/gl/dialog/WorldsDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Format.h"

namespace Game3 {
	namespace {
		constexpr float WIDTH = 150;
		constexpr float HEIGHT = 150;
	}

	namespace WorldUI {
		class Row: public Box {
			public:
				Row(WorldsDialog &dialog, std::filesystem::path path):
					Box(dialog.getUI(), 0.5, Orientation::Horizontal, 5, 0, Color{}),
					dialog(dialog),
					path(std::move(path)) {}

				void init() override {
					Box::init();
					setHorizontalExpand(true);
				}

			protected:
				WorldsDialog &dialog;
				std::filesystem::path path;
		};

		class DirectoryRow final: public Row {
			public:
				using Row::Row;

				void init() final {
					Row::init();
					auto self = getSelf();
					auto icon = make<Icon>(ui, selfScale);
					icon->setIconTexture(cacheTexture("resources/gui/folder.png"));
					icon->insertAtEnd(self);

					auto label = make<Label>(ui, selfScale, path.filename().string());
					label->insertAtEnd(self);

					label->setVerticalAlignment(Alignment::Center);
				}

				bool click(int button, int, int, Modifiers) final {
					if (button != LEFT_BUTTON) {
						return false;
					}

					dialog.currentPath = std::filesystem::canonical(path);
					dialog.populate();
					return true;
				}
		};

		class WorldRow final: public Row {
			public:
				using Row::Row;

				void init() final {
					Row::init();
					auto self = getSelf();
					auto icon = make<Icon>(ui, selfScale);
					icon->setIconTexture(cacheTexture("resources/gui/picture.png"));
					icon->insertAtEnd(self);

					auto label = make<Label>(ui, selfScale, path.filename().string());
					label->insertAtEnd(self);

					label->setVerticalAlignment(Alignment::Center);

					setOnClick([this](Widget &) {
						dialog.submit(path);
					});
				}
		};
	}

	WorldsDialog::WorldsDialog(UIContext &ui, float selfScale):
		DraggableDialog(ui, selfScale, WIDTH, HEIGHT) {
			setTitle("Worlds");
		}

	void WorldsDialog::init() {
		DraggableDialog::init();

		scroller = make<Scroller>(ui, selfScale);
		scroller->setExpand(true, true);

		vbox = make<Box>(ui, selfScale, Orientation::Vertical, 0);
		vbox->setHorizontalExpand(true);

		vbox->insertAtEnd(scroller);
		scroller->insertAtEnd(shared_from_this());

		recenter();
		populate();
	}

	void WorldsDialog::render(const RendererContext &renderers) {
		DraggableDialog::render(renderers);
		firstChild->render(renderers, bodyRectangle);
	}

	bool WorldsDialog::keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat) {
		if ((key == 'h' || key == 'H') && modifiers.onlyCtrl()) {
			showHidden = !showHidden;
			populate();
			return true;
		}

		return Widget::keyPressed(key, modifiers, is_repeat);
	}

	void WorldsDialog::rescale(float new_scale) {
		position.width = WIDTH * new_scale;
		position.height = HEIGHT * new_scale;
		Dialog::rescale(new_scale);
	}

	void WorldsDialog::submit(const std::filesystem::path &world_path) {
		signalSubmit(world_path);
		ui.removeDialog(getSelf());
	}

	void WorldsDialog::populate() {
		vbox->clearChildren();

		if (currentPath.empty()) {
			currentPath = std::filesystem::current_path() / "worlds";
		}

		std::set<std::filesystem::path> directories;
		std::set<std::filesystem::path> worlds;

		for (const auto &entry: std::filesystem::directory_iterator(currentPath)) {
			const std::filesystem::path &path = entry.path();

			if (showHidden && path.filename().string().starts_with('.')) {
				continue;
			}

			if (entry.is_directory()) {
				directories.emplace(path);
			} else if (entry.is_regular_file()) {
				if (path.extension() == ".db") {
					worlds.emplace(path);
				}
			}
		}

		make<WorldUI::DirectoryRow>(*this, currentPath / "..")->insertAtEnd(vbox);

		for (const std::filesystem::path &path: directories) {
			auto row = make<WorldUI::DirectoryRow>(*this, path);
			row->insertAtEnd(vbox);
		}

		for (const std::filesystem::path &path: worlds) {
			auto row = make<WorldUI::WorldRow>(*this, path);
			row->insertAtEnd(vbox);
		}

		scroller->scrollToTop();
	}
}
