#include "Painter.h"
#include "lineMath.h"
#include "color.h"

#include <boglfw/renderOpenGL/Shape2D.h>

void Painter::loadResources() {
	// load textures and shit
}

void Painter::paintArrow(ScreenPoint const& from, ScreenPoint const& to, int headSize, float headAperture, Color const& c) {
	ScreenPoint dir {to.x - from.x, to.y - from.y};
	if (dir.x == 0 && dir.y == 0)
		return;
	float angle = lineMath::pointDirection(dir) + M_PI;

	// draw arrow body:
	Shape2D::get()->drawLine(from, to, 0.f, c);
	// draw arrow head:
	ScreenPoint pL {to.x + cosf(angle+headAperture*0.5f) * headSize, to.y + sinf(angle+headAperture*0.5f) * headSize};
	ScreenPoint pR {to.x + cosf(angle-headAperture*0.5f) * headSize, to.y + sinf(angle-headAperture*0.5f) * headSize};
	Shape2D::get()->drawLine(to, pL, 0.f, c);
	Shape2D::get()->drawLine(to, pR, 0.f, c);
}
