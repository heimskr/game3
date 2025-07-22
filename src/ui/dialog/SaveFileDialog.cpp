#include "ui/dialog/SaveFileDialog.h"
#include "ui/widget/Icon.h"
#include "ui/widget/Scroller.h"
#include "ui/widget/TextInput.h"
#include "ui/UIContext.h"

namespace Game3 {
	void SaveFileDialog::init() {
		FileDialog::init();

		auto hbox = make<Box>(ui, selfScale, Orientation::Horizontal, 1, 0);

		filenameInput = make<TextInput>(ui, selfScale);
		filenameInput->setHorizontalExpand(true);
		filenameInput->onSubmit.connect([this](TextInput &, const UString &text) {
			submit(currentPath / text.raw());
		});

		auto ok_icon = make<Icon>(ui, selfScale, "resources/gui/yes.png");
		ok_icon->setFixedSize(10);
		ok_icon->setOnClick([this](Widget &) {
			submit(currentPath / filenameInput->getText().raw());
		});

		hbox->append(filenameInput);
		hbox->append(std::move(ok_icon));
		outerVbox->append(std::move(hbox));
	}

	void SaveFileDialog::selectFile(const std::filesystem::path &path) {
		filenameInput->setText(path.filename().string());
	}

	void SaveFileDialog::submit(const std::filesystem::path &path) {
		auto self = getSelf();
		ui.removeDialog(self);
		signalSubmit(path);
	}

	void SaveFileDialog::setFilename(UString filename) {
		filenameInput->setText(std::move(filename));
	}
}
