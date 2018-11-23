#include "SinglePlayerMenu.h"

SinglePlayerMenu::SinglePlayerMenu(glm::vec2 size)
	: VerticalMenu(size)
{
	std::vector<buttonDescriptor> buttons;
	buttons.push_back({
		"Start",
		[this](...) {
			onStart.trigger();
		}
	});

	buttons.push_back({
		"Back",
		[this](...) {
			onBack.trigger();
		}
	});

	setButtons(buttons);
}
