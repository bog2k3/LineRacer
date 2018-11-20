#include "MainMenu.h"

#include <boglfw/GUI/controls/Button.h>

MainMenu::MainMenu(glm::vec2 screenSize)
	: GuiContainerElement({0, 0}, screenSize)
{
	constexpr float buttonAspectRatio = 5.f;
	constexpr int maxButtonWidth = 400;
	constexpr float relativeButtonWidthPortrait = 0.8f; // of screen width
	constexpr float relativeButtonWidthLandscape = 0.25f; // of screen width
	const float relativeButtonWidth = screenSize.x > screenSize.y ? relativeButtonWidthLandscape : relativeButtonWidthPortrait;
	constexpr float buttonVerticalSpacing = 0.25f; // of button height
	constexpr float verticalExtent = 4.5; // of button height + button vertical spacing
	constexpr float maxVerticalCoverage = 0.8; // of screen height to leave some roome above and below

	int proportionalButtonWidth = screenSize.x * relativeButtonWidth;
	int actualButtonWidth = std::min(maxButtonWidth, proportionalButtonWidth);
	glm::vec2 buttonSize { actualButtonWidth, actualButtonWidth / buttonAspectRatio };
	int buttonYOffs = buttonSize.y * (1 + buttonVerticalSpacing);
	float actualVerticalCoverage = buttonYOffs * verticalExtent;
	float verticalScaling = 1.f;
	if (actualVerticalCoverage > maxVerticalCoverage * screenSize.y) {
		verticalScaling = maxVerticalCoverage * screenSize.y / actualVerticalCoverage;
		actualVerticalCoverage *= verticalScaling;
	}
	buttonSize *= verticalScaling;
	buttonYOffs *= verticalScaling;
	int buttonX = (screenSize.x - buttonSize.x) / 2;
	int buttonY = (screenSize.y - actualVerticalCoverage) / 2;

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
