#pragma once

#include <memory>

namespace Game3 {
	class Aligner;
	using AlignerPtr = std::shared_ptr<Aligner>;

	class AutocompleteDropdown;
	using AutocompleteDropdownPtr = std::shared_ptr<AutocompleteDropdown>;

	class Box;
	using BoxPtr = std::shared_ptr<Box>;

	class Button;
	using ButtonPtr = std::shared_ptr<Button>;

	class Checkbox;
	using CheckboxPtr = std::shared_ptr<Checkbox>;

	class ContextMenu;
	using ContextMenuPtr = std::shared_ptr<ContextMenu>;

	class CraftingSlider;
	using CraftingSliderPtr = std::shared_ptr<CraftingSlider>;

	class DialogueDisplay;
	using DialogueDisplayPtr = std::shared_ptr<DialogueDisplay>;

	class FullscreenWidget;
	using FullscreenWidgetPtr = std::shared_ptr<FullscreenWidget>;

	class Grid;
	using GridPtr = std::shared_ptr<Grid>;

	class Hotbar;
	using HotbarPtr = std::shared_ptr<Hotbar>;

	class Icon;
	using IconPtr = std::shared_ptr<Icon>;

	class IconButton;
	using IconButtonPtr = std::shared_ptr<IconButton>;

	class IntegerInput;
	using IntegerInputPtr = std::shared_ptr<IntegerInput>;

	class ItemSlot;
	using ItemSlotPtr = std::shared_ptr<ItemSlot>;

	class Label;
	using LabelPtr = std::shared_ptr<Label>;

	class ProgressBar;
	using ProgressBarPtr = std::shared_ptr<ProgressBar>;

	class Scroller;
	using ScrollerPtr = std::shared_ptr<Scroller>;

	class Slider;
	using SliderPtr = std::shared_ptr<Slider>;

	class Spacer;
	using SpacerPtr = std::shared_ptr<Spacer>;

	class SPinner;
	using SPinnerPtr = std::shared_ptr<SPinner>;

	class StatusEffectsDisplay;
	using StatusEffectsDisplayPtr = std::shared_ptr<StatusEffectsDisplay>;

	class TextInput;
	using TextInputPtr = std::shared_ptr<TextInput>;

	class Tooltip;
	using TooltipPtr = std::shared_ptr<Tooltip>;

	class Widget;
	using WidgetPtr = std::shared_ptr<Widget>;

}
