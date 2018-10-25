#include "WorldArea.h"
#include "Grid.h"
#include "color.h"

#include <boglfw/renderOpenGL/Shape2D.h>

WorldArea::WorldArea(Grid *g, GridPoint topLeft, GridPoint bottomRight)
	: grid_(g)
	, topLeft_(topLeft), bottomRight_(bottomRight) {
}

WorldArea::~WorldArea() {
}

void WorldArea::draw(Viewport*) {
	ScreenPoint sTopLeft = grid_->gridToScreen(topLeft_);
	ScreenPoint sBottomRight = grid_->gridToScreen(bottomRight_);

	const int thickness = 1;

	// left edge
	glm::vec2 pos {sTopLeft.x - thickness, sTopLeft.y - thickness};
	glm::vec2 sz {1 + 2*thickness, (sBottomRight.y - sTopLeft.y) + 2*thickness};
	Shape2D::get()->drawRectangleFilled(pos, 0, sz, Colors::GRID);
	// top edge
	pos = {sTopLeft.x - thickness, sTopLeft.y - thickness};
	sz = {(sBottomRight.x - sTopLeft.x) + 2*thickness, 1 + 2*thickness};
	Shape2D::get()->drawRectangleFilled(pos, 0, sz, Colors::GRID);
	// right edge
	pos = {sBottomRight.x - thickness, sTopLeft.y - thickness};
	sz = {1 + 2*thickness, (sBottomRight.y - sTopLeft.y) + 2*thickness};
	Shape2D::get()->drawRectangleFilled(pos, 0, sz, Colors::GRID);
	// bottom edge
	pos = {sTopLeft.x - thickness, sBottomRight.y - thickness};
	sz = {(sBottomRight.x - sTopLeft.x) + 2*thickness, 1 + 2*thickness};
	Shape2D::get()->drawRectangleFilled(pos, 0, sz, Colors::GRID);
}

bool WorldArea::containsPoint(WorldPoint const& wp) const {
	WorldPoint wTopLeft = grid_->gridToWorld(topLeft_);
	WorldPoint wBottomRight = grid_->gridToWorld(bottomRight_);
	return wp.x > wTopLeft.x && wp.y > wTopLeft.y
		&& wp.x < wBottomRight.x && wp.y < wBottomRight.y;
}
