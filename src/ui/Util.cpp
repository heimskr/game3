#include "ui/Util.h"

#include <GLFW/glfw3.h>

namespace Game3 {
	bool isModifierKey(int character) {
		switch (character) {
			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_RIGHT_SHIFT:
			case GLFW_KEY_LEFT_CONTROL:
			case GLFW_KEY_RIGHT_CONTROL:
			case GLFW_KEY_LEFT_ALT:
			case GLFW_KEY_RIGHT_ALT:
			case GLFW_KEY_LEFT_SUPER:
			case GLFW_KEY_RIGHT_SUPER:
				return true;

			default:
				return false;
		}
	}
}
