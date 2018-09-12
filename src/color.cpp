#include "color.h"

#include <SDL2/SDL_render.h>

void Color::set(SDL_Renderer* rend) const {
	SDL_SetRenderDrawColor(rend, r, g, b, a);
	if (a < 255)
		SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	else
		SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
}
