#include "ui/gl/dialog/LoadFileDialog.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
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
}
