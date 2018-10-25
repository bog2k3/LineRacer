#include "ButtonRound.h"

ButtonRound::ButtonRound(int centerX, int centerY, int radius)
	: Button(centerX-radius, centerY-radius, 2*radius, 2*radius) {
}

void ButtonRound::draw(Viewport* v) {
	Button::draw(v);
}

bool ButtonRound::containsPoint(int x, int y) {
	return false;
}
