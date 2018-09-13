#include "WorldArea.h"
#include "Grid.h"
#include "color.h"

#include <SDL2/SDL_render.h>

WorldArea::WorldArea(Grid *g, GridPoint topLeft, GridPoint bottomRight)
	: grid_(g)
	, topLeft_(topLeft), bottomRight_(bottomRight) {
}

WorldArea::~WorldArea() {
}

void WorldArea::render(SDL_Renderer* r) {
	ScreenPoint sTopLeft = grid_->gridToScreen(topLeft_);
	ScreenPoint sBottomRight = grid_->gridToScreen(bottomRight_);

	Colors::GRID.set(r);
	const int thickness = 1;

	// left edge
	SDL_Rect rc {
		sTopLeft.x - thickness, sTopLeft.y - thickness,
		1 + 2*thickness, (sBottomRight.y - sTopLeft.y) + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
	// top edge
	rc = {
		sTopLeft.x - thickness, sTopLeft.y - thickness,
		(sBottomRight.x - sTopLeft.x) + 2*thickness, 1 + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
	// right edge
	rc = {
		sBottomRight.x - thickness, sTopLeft.y - thickness,
		1 + 2*thickness, (sBottomRight.y - sTopLeft.y) + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
	// bottom edge
	rc = {
		sTopLeft.x - thickness, sBottomRight.y - thickness,
		(sBottomRight.x - sTopLeft.x) + 2*thickness, 1 + 2*thickness
	};
	SDL_RenderFillRect(r, &rc);
}

bool WorldArea::containsPoint(WorldPoint const& wp) const {
	WorldPoint wTopLeft = grid_->gridToWorld(topLeft_);
	WorldPoint wBottomRight = grid_->gridToWorld(bottomRight_);
	return wp.x > wTopLeft.x && wp.y > wTopLeft.y
		&& wp.x < wBottomRight.x && wp.y < wBottomRight.y;
}
