#include "graphics/Texture.h"
#include "ui/gl/dialog/FileDialog.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/Constants.h"

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
