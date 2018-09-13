#include "Track.h"
#include "Grid.h"
#include "transform.h"
#include "WorldArea.h"
#include "color.h"
#include "lineMath.h"

#include <SDL2/SDL_render.h>
#include <cassert>

static const int PARTITION_CELL_SPAN = 10;

Track::Track(Grid* grid, WorldArea* warea, float resolution)
	: grid_(grid), worldArea_(warea), partition_(*this, PARTITION_CELL_SPAN), resolution_(resolution)
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
		for (unsigned j=0; j<polyVertex_[i].size(); j++) {
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

		// if snapping close, draw the snap anchor
		if (polyVertex_[currentPolyIdx_].size() > 2 && floatingVertex_ == polyVertex_[currentPolyIdx_][0]) {
			const int snapAnchorRadius = 3;
			Colors::TRACK_SNAP.set(r);
			ScreenPoint anchor = floatingVertex_.toScreen(grid_->getTransform());
			SDL_Rect rc {anchor.x - snapAnchorRadius, anchor.y-snapAnchorRadius, 2*snapAnchorRadius+1, 2*snapAnchorRadius+1};
			SDL_RenderFillRect(r, &rc);
		}
	}
}

void Track::reset() {
	currentPolyIdx_ = 0;
	polyVertex_[0].clear();
	polyVertex_[1].clear();
	partition_.clear();
}

void Track::enableDesignMode(bool enable) {
	currentPolyIdx_ = 0;
	designMode_ = enable;
}

bool Track::checkCloseSnap() {
	// if the floating vertex comes to within a grid square of the starting point, snap it to close the loop
	if (polyVertex_[currentPolyIdx_].size() > 2
		&& floatingVertex_.distanceTo(polyVertex_[currentPolyIdx_][0]) < grid_->cellSize()
	) {
		floatingVertex_ = polyVertex_[currentPolyIdx_][0];
		return true;
	}
	return false;
}

bool Track::validateVertex() {
	// checks whether the floating vertex is within world's boundary
	// also when drawing second (inner) contour, it checks if the vertex is within the outer contour
	// also checks that the new line segment won't intersect any existing line segments

	// check world boundary:
	if (!worldArea_->containsPoint(floatingVertex_))
		return false;

	// check intersection:
	if (polyVertex_[currentPolyIdx_].size()) {
		auto prevVertex = polyVertex_[currentPolyIdx_].back();
		if (intersectLine(prevVertex, floatingVertex_, true))
			return false;
	}

	// check second contour to be inside first:
	if (currentPolyIdx_ == 1 && polyVertex_[1].size() == 0) {
		if (!pointInsidePolygon(floatingVertex_, 0))
			return false;
	}

	return true;
}

void Track::pushVertex() {
	int crtIndex = polyVertex_[currentPolyIdx_].size();
	partition_.addVertex({currentPolyIdx_, crtIndex}, floatingVertex_);
	if (crtIndex > 0)
		partition_.addSegment({currentPolyIdx_, {crtIndex-1, crtIndex}}, polyVertex_[currentPolyIdx_].back(), floatingVertex_);
	polyVertex_[currentPolyIdx_].push_back(floatingVertex_);
}

void Track::pointerMoved(float x, float y) {
	if (!designMode_)
		return;
	floatingVertex_.x = x;
	floatingVertex_.y = y;
	if (pointerPressed_
		&& polyVertex_[currentPolyIdx_].size()
		&& validateVertex()
	) {
		// live drawing
		float minDist = grid_->cellSize() / resolution_;
		if (floatingVertex_.distanceTo(polyVertex_[currentPolyIdx_].back()) >= minDist)
			pushVertex();
	} else
		checkCloseSnap();
}

void Track::pointerTouch(bool on, float x, float y) {
	if (!designMode_)
		return;
	pointerPressed_ = on;
	floatingVertex_.x = x;
	floatingVertex_.y = y;
	if (on && validateVertex()) {
		bool closed = checkCloseSnap();
		// add the next vertex
		pushVertex();

		if (closed) {
			if (currentPolyIdx_ == 0)
				currentPolyIdx_++;
			else
				enableDesignMode(false);
		}
	}
}

bool Track::intersectLine(WorldPoint const& p1, WorldPoint const& p2, bool skipLastSegment) const {
	WorldPoint topLeft {std::min(p1.x, p2.x), std::min(p1.y, p2.y)};
	WorldPoint bottomRight {std::max(p1.x, p2.x), std::max(p1.y, p2.y)};
	auto vertices = partition_.getVerticesInArea(topLeft, bottomRight);
	for (auto &v : vertices) {
		int polyIdx = v.first;
		int vIdx = v.second;
		if (skipLastSegment && polyIdx == currentPolyIdx_ && (unsigned)vIdx+1 == polyVertex_[currentPolyIdx_].size())
			continue; // we're not checking against last segment because that one is connected
		if (vIdx > 0) {
			// check line before vertex
			if (lineMath::segmentIntersect(p1, p2, polyVertex_[polyIdx][vIdx-1], polyVertex_[polyIdx][vIdx]))
				return true;
		}
		if (skipLastSegment && polyIdx == currentPolyIdx_ && (unsigned)vIdx+2 == polyVertex_[currentPolyIdx_].size())
			continue; // we're not checking against last segment because that one is connected
		if ((unsigned)vIdx + 1 < polyVertex_[polyIdx].size()) {
			// check line following vertex
			if (lineMath::segmentIntersect(p1, p2, polyVertex_[polyIdx][vIdx], polyVertex_[polyIdx][vIdx+1]))
				return true;
		}
	}
	return false;
}

bool Track::intersectLine(GridPoint const& p1, GridPoint const& p2) const {
	return intersectLine(grid_->gridToWorld(p1), grid_->gridToWorld(p2), false);
}

bool Track::pointInsidePolygon(WorldPoint const& p, int polyIndex) const {
	assert(polyIndex >= 0 && polyIndex <= 1);
	if (polyVertex_[polyIndex].size() < 3)
		return false;
	int clockwiseness = lineMath::clockwiseness(polyVertex_[polyIndex].data(), polyVertex_[polyIndex].size()-1);	//-1 because last vertex is a duplicate of the first
	int sidedness = 0;
	for (unsigned i=0; i<polyVertex_[polyIndex].size()-1; i++) // -1 because we look one vertex ahead
		sidedness += lineMath::orientation(polyVertex_[polyIndex][i], polyVertex_[polyIndex][i+1], p);
	// if the polygon's clockwiseness has a different sign than the point's sidedness, then the point is outside
	if (clockwiseness * sidedness < 0)
		return false;
	// more than half of the edges must have the same orientation towards the test-point as the polygon's clockwiseness
	return abs(sidedness) > (polyVertex_[polyIndex].size()-1) / 2.f;

	THIS IS FLAWED !!!
}
