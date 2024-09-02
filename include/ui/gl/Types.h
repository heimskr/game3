#pragma once

namespace Game3 {
	enum class Orientation: char {Vertical, Horizontal};
	enum class SizeRequestMode: char {HeightForWidth, WidthForHeight, ConstantSize, Expansive};
	enum class Alignment: char {Start, Middle, End};
}
