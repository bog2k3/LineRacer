#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class MainMenu : public GuiContainerElement {
public:
	MainMenu(glm::vec2 size);
	~MainMenu() override {}

	Event<void()> onSinglePlayer;
	Event<void()> onJoinMulti;
	Event<void()> onHostMulti;
	Event<void()> onExit;
private:
};
