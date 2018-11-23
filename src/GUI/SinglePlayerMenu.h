#pragma once

#include "VerticalMenu.h"
#include <boglfw/utils/Event.h>

class SinglePlayerMenu : public VerticalMenu {
public:
	SinglePlayerMenu(glm::vec2 size);
	~SinglePlayerMenu() override {}

	Event<void()> onStart;
	Event<void()> onBack;
private:
};
