#include "ui/gl/dialog/SaveFileDialog.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	void SaveFileDialog::init() {
		FileDialog::init();

		auto vbox = make<Box>(ui, selfScale, Orientation::Vertical, 0);

		filenameInput = make<TextInput>(ui, selfScale);
		filenameInput->onSubmit.connect([this](TextInput &, const UString &text) {
			submit(currentPath / text.raw());
		});

		remove(scroller);
		vbox->append(scroller);
		vbox->append(filenameInput);
		vbox->insertAtEnd(getSelf());
	}

	void SaveFileDialog::selectFile(const std::filesystem::path &path) {
		filenameInput->setText(path.filename().string());
	}

	void SaveFileDialog::submit(const std::filesystem::path &path) {
		auto self = getSelf();
		ui.removeDialog(self);
		signalSubmit(path);
	}
}
