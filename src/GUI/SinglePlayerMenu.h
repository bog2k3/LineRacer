#pragma once

#include "VerticalMenu.h"
#include <boglfw/utils/Event.h>

#include <memory>

class SinglePlayerMenu : public VerticalMenu {
public:
	SinglePlayerMenu(glm::vec2 size);
	~SinglePlayerMenu() override {}

	// Event<void()> onSinglePlayer;
	Event<void()> onBack;
private:
};
