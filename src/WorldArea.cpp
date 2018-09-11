#include "WorldArea.h"
#include "Grid.h"

#include <SDL2/SDL_render.h>

WorldArea::WorldArea(Grid *g, GridPoint topLeft, GridPoint bottomRight)
	: grid_(g)
	, gX_(topLeft.x), gY_(topLeft.y)
	, gW_(bottomRight.x - topLeft.x), gH_(bottomRight.y - topLeft.y) {
}

WorldArea::~WorldArea() {
}

void WorldArea::render(SDL_Renderer* r, Transform const& tr) {
	ScreenPoint topLeft = grid_->gridToScreen({gX_, gY_}, tr);
	ScreenPoint bottomRight = grid_->gridToScreen({gX_+gW_, gY_+gH_}, tr);

	SDL_SetRenderDrawColor(r, 190, 210, 255, SDL_ALPHA_OPAQUE);
	const int thickness = 1;

	// left edge
	SDL_Rect rc {
		topLeft.x - thickness, topLeft.y - thickness,
		1 + 2*thickness, (bottomRight.y - topLeft.y) + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
	// top edge
	rc = {
		topLeft.x - thickness, topLeft.y - thickness,
		(bottomRight.x - topLeft.x) + 2*thickness, 1 + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
	// right edge
	rc = {
		bottomRight.x - thickness, topLeft.y - thickness,
		1 + 2*thickness, (bottomRight.y - topLeft.y) + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
	// bottom edge
	rc = {
		topLeft.x - thickness, bottomRight.y - thickness,
		(bottomRight.x - topLeft.x) + 2*thickness, 1 + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
}
