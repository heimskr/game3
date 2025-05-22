#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/dialog/FileDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	namespace FileChooser {
		Row::Row(FileDialog &dialog, std::filesystem::path path, TexturePtr iconTexture):
			Box(dialog.getUI(), 0.6, Orientation::Horizontal, 5, 0, Color{}),
			dialog(dialog),
			path(std::move(path)),
			iconTexture(std::move(iconTexture)) {}

		void Row::init() {
			Box::init();
			setHorizontalExpand(true);
		}

		void DirectoryRow::init() {
			Row::init();
			auto self = getSelf();
			auto icon = make<Icon>(ui, selfScale);
			icon->setIconTexture(iconTexture);
			icon->insertAtEnd(self);
			auto label = make<Label>(ui, selfScale * 1.25, path.filename().string());
			label->setVerticalAlignment(Alignment::Center);
			label->insertAtEnd(self);
		}

		bool DirectoryRow::click(int button, int, int, Modifiers) {
			if (button != LEFT_BUTTON) {
				return false;
			}

			dialog.selectDirectory(path);
			return true;
		}

		void FileRow::init() {
			Row::init();
			auto self = getSelf();
			auto icon = make<Icon>(ui, selfScale);
			icon->setIconTexture(iconTexture);
			icon->insertAtEnd(self);
			auto label = make<Label>(ui, selfScale * 1.25, path.filename().string());
			label->setVerticalAlignment(Alignment::Center);
			label->insertAtEnd(self);
		}

		bool FileRow::click(int button, int, int, Modifiers) {
			if (button != LEFT_BUTTON) {
				return false;
			}

			dialog.selectFile(path);
			return true;
		}
	}

	FileDialog::FileDialog(UIContext &ui, float selfScale, UString title, int width, int height):
		DraggableDialog(ui, selfScale, width, height) {
			setTitle(std::move(title));
		}

	void FileDialog::init() {
		DraggableDialog::init();

		currentPath = std::filesystem::current_path() / "worlds";

		scroller = make<Scroller>(ui, selfScale);
		scroller->setExpand(true, true);

		outerVbox = make<Box>(ui, selfScale, Orientation::Vertical, 1, 0.666);
		outerVbox->setExpand(true, true);

		header = make<Box>(ui, selfScale, Orientation::Horizontal, 0);
		header->setExpand(true, false);

		upIcon = make<Icon>(ui, selfScale);
		upIcon->setIconTexture(getTexture(std::filesystem::directory_entry(currentPath / "..")));
		upIcon->setFixedSize(8);
		upIcon->setOnClick([this](Widget &) {
			std::filesystem::path parent = currentPath.parent_path();
			if (currentPath != parent) {
				currentPath = std::move(parent);
				populate();
			}
		});

		pathScroller = make<Scroller>(ui, selfScale);
		pathScroller->setExpand(Expansion::Expand, Expansion::Shrink);

		pathLabel = make<Label>(ui, selfScale, currentPath.string());
		pathLabel->setMayWrap(false);
		pathLabel->setExpand(true, false);

		entryList = make<Box>(ui, selfScale, Orientation::Vertical, 0);
		entryList->setExpand(true, true);

		upIcon->insertAtEnd(header);
		pathLabel->insertAtEnd(pathScroller);
		pathScroller->insertAtEnd(header);
		outerVbox->append(header);
		outerVbox->append(entryList);
		scroller->setChild(outerVbox);
		scroller->insertAtEnd(shared_from_this());

		recenter();
		populate();
	}

	bool FileDialog::keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat) {
		if ((key == 'h' || key == 'H') && modifiers.onlyCtrl()) {
			showHidden = !showHidden;
			populate();
			return true;
		}

		DraggableDialog::keyPressed(key, modifiers, is_repeat);
		return true;
	}

	void FileDialog::rescale(float new_scale) {
		position.width = dialogWidth * new_scale;
		position.height = dialogHeight * new_scale;
		DraggableDialog::rescale(new_scale);
	}

	void FileDialog::selectDirectory(const std::filesystem::path &path) {
		currentPath = std::filesystem::canonical(path);
		populate();
	}

	void FileDialog::populate() {
		entryList->clearChildren();

		pathLabel->setText(currentPath.string());

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

		for (const std::filesystem::directory_entry &entry: directories) {
			auto row = make<FileChooser::DirectoryRow>(*this, entry.path(), getTexture(entry));
			row->insertAtEnd(entryList);
		}

		for (const std::filesystem::directory_entry &entry: worlds) {
			auto row = make<FileChooser::FileRow>(*this, entry.path(), getTexture(entry));
			row->insertAtEnd(entryList);
		}

		scroller->scrollToTop();
		scroller->remeasure(ui.getRenderers(0));
	}

	bool FileDialog::filter(const std::filesystem::directory_entry &) const {
		return true;
	}

	TexturePtr FileDialog::getTexture(const std::filesystem::directory_entry &entry) const {
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
