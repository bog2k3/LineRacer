#include "Track.h"
#include "Grid.h"
#include "transform.h"
#include "WorldArea.h"
#include "color.h"

#include <SDL2/SDL_render.h>
#include <cmath>

Track::Track(Grid* grid, WorldArea* warea, float resolution)
	: grid_(grid), worldArea_(warea), resolution_(resolution)
{
}

Track::~Track() {
}

void Track::render(SDL_Renderer* r) {
	Colors::TRACK.set(r);
	static std::vector<SDL_Point> vPoints;
	vPoints.reserve(std::max(polyVertex_[0].size(), polyVertex_[1].size()));
	for (int i=0; i<2; i++) {
		if (polyVertex_[i].size() < 2)
			continue;
		vPoints.clear();
		for (int j=0; j<polyVertex_[i].size(); j++) {
			ScreenPoint sp = polyVertex_[i][j].toScreen(grid_->getTransform());
			vPoints.push_back({sp.x, sp.y});
		}
		SDL_RenderDrawLines(r, vPoints.data(), vPoints.size());
	}
	if (designMode_ && polyVertex_[currentPolyIdx_].size() > 0) {
		ScreenPoint p1 = polyVertex_[currentPolyIdx_].back().toScreen(grid_->getTransform());
		ScreenPoint p2 = floatingVertex_.toScreen(grid_->getTransform());
		Colors::TRACK_TRANSP.set(r);
		SDL_RenderDrawLine(r, p1.x, p1.y, p2.x, p2.y);
	}
}

void Track::reset() {
	currentPolyIdx_ = 0;
	polyVertex_[0].clear();
	polyVertex_[0].clear();
}

void Track::enableDesignMode(bool enable) {
	designMode_ = enable;
}

void Track::pointerMoved(float x, float y) {
	floatingVertex_.x = x;
	floatingVertex_.y = y;
	if (pointerPressed_) {
		// ...
	}
}

void Track::pointerTouch(bool on, float x, float y) {
	if (!designMode_)
		return;
	pointerPressed_ = on;
	floatingVertex_.x = x;
	floatingVertex_.y = y;
	if (on && worldArea_->containsPoint(floatingVertex_)) {
		// add the next vertex
		polyVertex_[currentPolyIdx_].push_back(floatingVertex_);
	}
}

bool Track::intersectLine(GridPoint const& p1, GridPoint const& p2) const {
	// TODO should partition our line segments into a BSP to speed up computation
}
