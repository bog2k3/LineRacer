#include "ButtonRound.h"

#include <SDL2/SDL_render.h>

ButtonRound::ButtonRound(int centerX, int centerY, int radius)
	: Button(centerX-radius, centerY-radius, 2*radius, 2*radius) {
}

void ButtonRound::render(SDL_Renderer* r) {
	Button::render(r);
}

bool ButtonRound::containsPoint(int x, int y) {
	return false;
}
