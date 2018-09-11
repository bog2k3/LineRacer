#include "Button.h"

#include <SDL2/SDL_render.h>

Button::Button(int x, int y, int w, int h)
	: GUIElement()
{
	area_ = {x, y, w, h};
	text_ = "Button";
}

void Button::render(SDL_Renderer* r) {
	// draw border
	SDL_Rect rc { area_.x, area_.y, area_.w, area_.h };
	//SDL_SetRenderDrawColor(r, 20, 150, 170, 255);
	SDL_SetRenderDrawColor(r, 5, 100, 110, 255);
	SDL_RenderDrawRect(r, &rc);

	// draw fill
	rc.x++; rc.y++;
	rc.w -= 2; rc.h -= 2;
	if (isHover()) {
		if (isPressed())
			SDL_SetRenderDrawColor(r, 25, 145, 215, 255);
		else
			SDL_SetRenderDrawColor(r, 135, 225, 240, 255);
	} else
		SDL_SetRenderDrawColor(r, 80, 215, 235, 255);
	SDL_RenderFillRect(r, &rc);

	// draw text
	//...
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
