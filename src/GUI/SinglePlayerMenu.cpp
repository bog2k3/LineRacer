#include "SinglePlayerMenu.h"

#include <boglfw/GUI/controls/RoundButton.h>

static const float margin = 0.1f; // of screen size

SinglePlayerMenu::SinglePlayerMenu(glm::vec2 size)
	: GuiContainerElement(size * margin, size * (1 - 2*margin))
{
	setTransparentBackground(true);
	/*std::vector<buttonDescriptor> buttons;
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

	setButtons(buttons);*/
}
