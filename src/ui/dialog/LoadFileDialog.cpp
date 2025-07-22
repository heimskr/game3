#include "ui/dialog/LoadFileDialog.h"
#include "ui/UIContext.h"

namespace Game3 {
	void LoadFileDialog::selectFile(const std::filesystem::path &path) {
		submit(path);
	}

	void LoadFileDialog::submit(const std::filesystem::path &path) {
		auto self = getSelf();
		ui.removeDialog(self);
		signalSubmit(path);
	}
}
