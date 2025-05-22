#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "ui/gl/dialog/FileDialog.h"
#include "ui/gl/dialog/LoadFileDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Format.h"

namespace Game3 {
	void LoadFileDialog::init() {
		FileDialog::init();

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

	void LoadFileDialog::selectFile(const std::filesystem::path &path) {
		submit(path);
	}

	void LoadFileDialog::selectDirectory(const std::filesystem::path &path) {
		currentPath = std::filesystem::canonical(path);
		populate();
	}

	void LoadFileDialog::submit(const std::filesystem::path &path) {
		auto self = getSelf();
		ui.removeDialog(self);
		signalSubmit(path);
	}

	void LoadFileDialog::populate() {
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
}
