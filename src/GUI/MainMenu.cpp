#include "MainMenu.h"

#include <boglfw/GUI/controls/Button.h>

static const float margin = 0.1f; // of screen size

MainMenu::MainMenu(glm::vec2 screenSize)
	: GuiContainerElement(screenSize * margin, screenSize * (1 - 2*margin))
{
	constexpr float buttonAspectRatio = 5.f;
	constexpr int maxButtonWidth = 400;
	constexpr float relativeButtonWidthPortrait = 0.8f; // of menu width
	constexpr float relativeButtonWidthLandscape = 0.25f; // of menu width
	const float relativeButtonWidth = screenSize.x > screenSize.y ? relativeButtonWidthLandscape : relativeButtonWidthPortrait;
	constexpr float buttonVerticalSpacing = 0.25f; // of button height
	constexpr float verticalExtent = 4.5; // of button height + button vertical spacing
	constexpr float maxVerticalCoverage = 0.8; // of menu height to leave some roome above and below

	int proportionalButtonWidth = getSize().x * relativeButtonWidth;
	int actualButtonWidth = std::min(maxButtonWidth, proportionalButtonWidth);
	glm::vec2 buttonSize { actualButtonWidth, actualButtonWidth / buttonAspectRatio };
	int buttonYOffs = buttonSize.y * (1 + buttonVerticalSpacing);
	float actualVerticalCoverage = buttonYOffs * verticalExtent;
	float verticalScaling = 1.f;
	if (actualVerticalCoverage > maxVerticalCoverage * getSize().y) {
		verticalScaling = maxVerticalCoverage * getSize().y / actualVerticalCoverage;
		actualVerticalCoverage *= verticalScaling;
	}
	buttonSize *= verticalScaling;
	buttonYOffs *= verticalScaling;
	int buttonX = (getSize().x - buttonSize.x) / 2;
	int buttonY = (getSize().y - actualVerticalCoverage) / 2;

	auto pB = std::make_shared<Button>(glm::vec2{buttonX, buttonY}, buttonSize, "Single Player");
	pB->onClick.add([this](Button* b) {
		onSinglePlayer.trigger();
	});
	addElement(pB);

	pB = std::make_shared<Button>(glm::vec2{buttonX, buttonY + 1*buttonYOffs}, buttonSize, "Host Multi Player");
	pB->onClick.add([this](Button* b) {
		onHostMulti.trigger();
	});
	addElement(pB);

	pB = std::make_shared<Button>(glm::vec2{buttonX, buttonY + 2*buttonYOffs}, buttonSize, "Join Multi Player");
	pB->onClick.add([this](Button* b) {
		onJoinMulti.trigger();
	});
	addElement(pB);

	pB = std::make_shared<Button>(glm::vec2{buttonX, buttonY + 3.5*buttonYOffs}, buttonSize, "Exit");
	pB->onClick.add([this](Button* b) {
		onExit.trigger();
	});
	addElement(pB);
}
