#include "Button.h"
#include "../color.h"

#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>

Button::Button(int x, int y, int w, int h)
	: GUIElement()
{
	area_ = {x, y, w, h};
	text_ = "Button";
}

void Button::draw(Viewport*) {
	// draw border
	Shape2D::get()->drawRectangle({area_.x, area_.y}, 0, {area_.w, area_.h}, Colors::BUTTON_BORDER);

	// draw fill
	const Color *c = isHover() ? isPressed() ? &Colors::BUTTON_FILL_PRESSED : &Colors::BUTTON_FILL_HOVER : &Colors::BUTTON_FILL;
	Shape2D::get()->drawRectangleFilled({area_.x+1, area_.y+1}, 0, {area_.w-2, area_.h-2}, *c);

	// draw text
	auto textRc = GLText::get()->getTextRect(text_, 16);
	glm::vec2 textPos = {area_.x + area_.w / 2 - textRc.x / 2, area_.y + area_.h / 2 - textRc.y /2};
	GLText::get()->print(text_, textPos, 0, 16, Colors::BUTTON_TEXT);
}

void Button::setText(std::string text) {
	text_ = text;
}

void Button::setIcon(...) {
	//...
}

void Button::setAction(std::function<void(Button*)> callback) {
	action_ = callback;
}

bool Button::onTouchEnd(int x, int y) {
	pressed_ = false;
	if (hover_ && action_)
		action_(this);
	return true;
}
