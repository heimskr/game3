#include "graphics/Texture.h"
#include "ui/gl/dialog/FileChooserDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Format.h"

namespace Game3 {
	namespace FileChooser {
		class Row: public Box {
			public:
				Row(FileChooserDialog &dialog, std::filesystem::path path, TexturePtr iconTexture):
					Box(dialog.getUI(), 0.6, Orientation::Horizontal, 5, 0, Color{}),
					dialog(dialog),
					path(std::move(path)),
					iconTexture(std::move(iconTexture)) {}

				void init() override {
					Box::init();
					setHorizontalExpand(true);
				}

			protected:
				FileChooserDialog &dialog;
				std::filesystem::path path;
				TexturePtr iconTexture;
		};

		class DirectoryRow final: public Row {
			public:
				using Row::Row;

				void init() final {
					Row::init();
					auto self = getSelf();
					auto icon = make<Icon>(ui, selfScale);
					std::string filename = path.filename().string();
					icon->setIconTexture(iconTexture);
					icon->insertAtEnd(self);
					if (path.filename() != "..") {
						auto label = make<Label>(ui, selfScale * 1.25, std::move(filename));
						label->setVerticalAlignment(Alignment::Center);
						label->insertAtEnd(self);
					}
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

		class FileRow final: public Row {
			public:
				using Row::Row;

				void init() final {
					Row::init();
					auto self = getSelf();
					auto icon = make<Icon>(ui, selfScale);
					icon->setIconTexture(iconTexture);
					icon->insertAtEnd(self);

					auto label = make<Label>(ui, selfScale * 1.25, path.filename().string());
					label->setVerticalAlignment(Alignment::Center);
					label->insertAtEnd(self);
				}

				bool click(int button, int, int, Modifiers) final {
					if (button != LEFT_BUTTON) {
						return false;
					}

					dialog.submit(path);
					return true;
				}
		};
	}

	FileChooserDialog::FileChooserDialog(UIContext &ui, float selfScale, UString title, int width, int height):
		DraggableDialog(ui, selfScale, width, height) {
			setTitle(std::move(title));
		}

	void FileChooserDialog::init() {
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

	void FileChooserDialog::render(const RendererContext &renderers) {
		DraggableDialog::render(renderers);
		firstChild->render(renderers, bodyRectangle);
	}

	bool FileChooserDialog::keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat) {
		if ((key == 'h' || key == 'H') && modifiers.onlyCtrl()) {
			showHidden = !showHidden;
			populate();
			return true;
		}

		Dialog::keyPressed(key, modifiers, is_repeat);
		return true;
	}

	void FileChooserDialog::rescale(float new_scale) {
		position.width = dialogWidth * new_scale;
		position.height = dialogHeight * new_scale;
		Dialog::rescale(new_scale);
	}

	void FileChooserDialog::submit(const std::filesystem::path &world_path) {
		auto self = getSelf();
		ui.removeDialog(self);
		signalSubmit(world_path);
	}

	void FileChooserDialog::populate() {
		vbox->clearChildren();

		if (currentPath.empty()) {
			currentPath = std::filesystem::current_path() / "worlds";
		}

		std::set<std::filesystem::directory_entry> directories;
		std::set<std::filesystem::directory_entry> worlds;

		for (const auto &entry: std::filesystem::directory_iterator(currentPath, std::filesystem::directory_options::skip_permission_denied)) {
			if (showHidden && entry.path().filename().string().starts_with('.')) {
				continue;
			}

			if (entry.is_directory()) {
				directories.emplace(entry);
			} else if (entry.is_regular_file()) {
				if (filter(entry)) {
					worlds.emplace(entry);
				}
			}
		}

		std::filesystem::path parent = currentPath / "..";
		make<FileChooser::DirectoryRow>(*this, parent, getTexture(std::filesystem::directory_entry(parent)))->insertAtEnd(vbox);

		for (const std::filesystem::directory_entry &entry: directories) {
			auto row = make<FileChooser::DirectoryRow>(*this, entry.path(), getTexture(entry));
			row->insertAtEnd(vbox);
		}

		for (const std::filesystem::directory_entry &entry: worlds) {
			auto row = make<FileChooser::FileRow>(*this, entry.path(), getTexture(entry));
			row->insertAtEnd(vbox);
		}

		scroller->scrollToTop();
	}

	bool FileChooserDialog::filter(const std::filesystem::directory_entry &) const {
		return true;
	}

	TexturePtr FileChooserDialog::getTexture(const std::filesystem::directory_entry &entry) const {
		if (entry.is_regular_file()) {
			return cacheTexture("resources/gui/file.png");
		}

		if (entry.is_directory()) {
			if (entry.path().filename() == "..") {
				return cacheTexture("resources/gui/up.png");
			}
			return cacheTexture("resources/gui/folder.png");
		}

		return cacheTexture("resources/gui/question_mark.png");
	}
}
