#include "Painter.h"
#include "lineMath.h"

#include <SDL2/SDL_render.h>
//#include <SDL2/SDL_rect.h>

static SDL_Renderer* renderer = nullptr;

void Painter::setRenderer(SDL_Renderer* r) {
	renderer = r;
}

void Painter::loadResources() {
	// load textures and shit
}

void Painter::paintArrow(ScreenPoint const& from, ScreenPoint const& to, int headSize, float headAperture) {
	ScreenPoint dir {to.x - from.x, to.y - from.y};
	float angle = lineMath::pointDirection(dir) + M_PI;

	// draw arrow body:
	SDL_RenderDrawLine(renderer, from.x, from.y, to.x, to.y);
	// draw arrow head:
	ScreenPoint pL {to.x + cosf(angle+headAperture*0.5f) * headSize, to.y + sinf(angle+headAperture*0.5f) * headSize};
	ScreenPoint pR {to.x + cosf(angle-headAperture*0.5f) * headSize, to.y + sinf(angle-headAperture*0.5f) * headSize};
	SDL_RenderDrawLine(renderer, to.x, to.y, pL.x, pL.y);
	SDL_RenderDrawLine(renderer, to.x, to.y, pR.x, pR.y);
}
