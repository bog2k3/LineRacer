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
	if (designMode_ && designStep_ == TrackDesignStep::DRAW && polyVertex_[currentPolyIdx_].size() > 0) {
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
	if (designMode_ && designStep_ == TrackDesignStep::STARTLINE && startLine_.isValid) {
		Colors::STARTLINE.set(r);
		auto s1 = startLine_.p1.toScreen(grid_->getTransform());
		auto s2 = startLine_.p2.toScreen(grid_->getTransform());
		SDL_RenderDrawLine(r, s1.x, s1.y, s2.x, s2.y);
		for (auto &spos : startLine_.startPositions) {
			auto p1 = grid_->gridToScreen(spos.position);
			auto p2 = grid_->gridToScreen({spos.position.x + spos.direction.first, spos.position.y + spos.direction.second});
			SDL_RenderDrawLine(r, p1.x, p1.y, p2.x, p2.y);
		}
	}
	
#if 0 || DEBUG_CODE_TO_TEST_POINT_INSIDE_POLYGON
	if (!designMode_ && polyVertex_[1].size()) {
		static std::vector<std::pair<WorldPoint, bool>> vPoints;
		if (vPoints.size() < 10000) {
			WorldPoint tl = grid_->gridToWorld(worldArea_->topLeft());
			WorldPoint br = grid_->gridToWorld(worldArea_->bottomRight());
			WorldPoint wp{rand() / (float)RAND_MAX * (br.x-tl.x) + tl.x, rand() / (float)RAND_MAX * (br.y-tl.y) + tl.y};
			bool green = pointInsidePolygon(wp, 0) && !pointInsidePolygon(wp, 1);
			vPoints.push_back({wp, green});
		}
		//SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
		for (auto &p : vPoints) {
			if (p.second)
				SDL_SetRenderDrawColor(r, 0, 255, 0, 255);
			else
				SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
			SDL_Rect rc{p.first.x, p.first.y, 2, 2};
			SDL_RenderDrawRect(r, &rc);
		}
		//SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
	}
#endif
}

void Track::reset() {
	currentPolyIdx_ = 0;
	polyVertex_[0].clear();
	polyVertex_[1].clear();
	partition_.clear();
	startLine_.isValid = false;
}

void Track::enableDesignMode(bool enable) {
	currentPolyIdx_ = 0;
	designMode_ = enable;
	designStep_ = TrackDesignStep::DRAW;
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

void Track::updateStartLine() {
	startLine_.isValid = false;
	GridPoint touch = grid_->worldToGrid(floatingVertex_);
	if (!pointInsidePolygon(grid_->gridToWorld(touch), 0) || pointInsidePolygon(grid_->gridToWorld(touch), 1)) {
		return;
	}
	
	// sweep all 4 possible directions and chose the one that intersects both polygons at the shortest distance
	std::pair<int, int> directions[] {
		{1, 0}, {0, 1}, {1, 1}, {-1, 1}
	};
	struct {
		int steps;
		float distance;
		int polyId;
		WorldPoint p1;
		WorldPoint p2;
	} intersections[4];
	for (int i=0; i<4; i++) {
		intersections[i].steps = 0;
		for (int j=-1; j<=1; j+=2) {
			// when j==1 we sweep positive, j==-1 we sweep negative
			GridPoint p = touch;
			int steps = 0;
			int lastPolyIdx = -1;
			do {
				p.x += directions[i].first * j;
				p.y += directions[i].second * j;
				intersections[i].steps++;
			} while (!intersectLine(touch, p, j==-1 ? &intersections[i].p1 : &intersections[i].p2, &lastPolyIdx));
			if (j==-1)
				intersections[i].polyId = lastPolyIdx;
			else if (intersections[i].polyId == lastPolyIdx) {
				intersections[i].steps = 10000; // we don't count this one if it doesn't intersect both polygons
			}
		}
		intersections[i].distance = intersections[i].steps;
		if (i>=2)
			 intersections[i].distance *= 1.41f;	// because last two are diagonals
	}
	int imin = 0;
	for (int i=1; i<4; i++) {
		if (intersections[i].distance < intersections[imin].distance)
			imin = i;
	}
	if (intersections[imin].steps == 10000)
		return; // no valid start line found
	startLine_.isValid = true;
	startLine_.p1 = intersections[imin].p1;
	startLine_.p2 = intersections[imin].p2;
	startLine_.startPositions.clear();
	GridPoint p = grid_->worldToGrid(startLine_.p1);
	for (int i=0; i<intersections[imin].steps; p.x+=directions[imin].first, p.y+=directions[imin].second, i++) {
		if (!pointInsidePolygon(grid_->gridToWorld(p), 0) || pointInsidePolygon(grid_->gridToWorld(p), 1))
			continue;
		bool CW = lineMath::orientation(intersections[imin].p1, intersections[imin].p2, floatingVertex_) == 1;
		int dirx = directions[imin].second * (CW ? 1 : -1);
		int diry = directions[imin].first * (CW ? -1 : 1);
		if (!intersectLine(p, {p.x + dirx, p.y + diry})) {
			// this is a valid start position and direction
			startLine_.startPositions.push_back({p, {dirx, diry}});
		}
	}
}

void Track::saveStartLine() {

}

void Track::pointerMoved(float x, float y) {
	if (!designMode_)
		return;
	floatingVertex_.x = x;
	floatingVertex_.y = y;
	switch (designStep_) {
	case TrackDesignStep::DRAW:
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
		break;
	case TrackDesignStep::STARTLINE:
		updateStartLine();
		break;
	}
}

void Track::pointerTouch(bool on, float x, float y) {
	if (!designMode_)
		return;
	pointerPressed_ = on;
	floatingVertex_.x = x;
	floatingVertex_.y = y;
	switch (designStep_) {
	case TrackDesignStep::DRAW:
		if (on && validateVertex()) {	// TODO: bug here - detecting false collision with first line segment (which should indeed touch at the end)
			bool closed = checkCloseSnap();
			// add the next vertex
			pushVertex();

			if (closed) {
				if (currentPolyIdx_ == 0)
					currentPolyIdx_++;
				else
					designStep_ = TrackDesignStep::STARTLINE; // go to next step
			}
		}
		break;
	case TrackDesignStep::STARTLINE:
		if (on)
			updateStartLine();
		else if (startLine_.isValid)
			enableDesignMode(false);
		break;
	}
}

bool Track::intersectLine(WorldPoint const& p1, WorldPoint const& p2, bool skipLastSegment, WorldPoint* out_point, int *out_polyIndex) const {
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
			if (lineMath::segmentIntersect(p1, p2, polyVertex_[polyIdx][vIdx], polyVertex_[polyIdx][vIdx+1])) {
				if (out_point)
					*out_point = lineMath::intersectionPoint(p1, p2, polyVertex_[polyIdx][vIdx], polyVertex_[polyIdx][vIdx+1]);
				if (out_polyIndex)
					*out_polyIndex = polyIdx;
				return true;
			}
		}
	}
	return false;
}

bool Track::intersectLine(GridPoint const& p1, GridPoint const& p2, WorldPoint* out_point, int *out_polyIndex) const {
	return intersectLine(grid_->gridToWorld(p1), grid_->gridToWorld(p2), false, out_point, out_polyIndex);
}

bool Track::pointInsidePolygon(WorldPoint const& p, int polyIndex, float* out_winding) const {
	assert(polyIndex >= 0 && polyIndex <= 1);
	if (polyVertex_[polyIndex].size() < 3)
		return false;
	auto lineSpansPoint = [](WorldPoint const& l1, WorldPoint const& l2, WorldPoint const& p) -> bool {
		if ((l1.x - p.x) * (l2.x - p.x) < 0)
			return true;
		if ((l1.y - p.y) * (l2.y - p.y) < 0)
			return true;
		return false;
	};
	auto &verts = polyVertex_[polyIndex];
	unsigned n = verts.size() - 1; // -1 because the last vertex is a duplicate of the first
	double winding = 0.0;
	bool polyCW = lineMath::clockwiseness(verts.data(), n) > 0;
	for (unsigned i=0; i<n; i++) {
		//if (!lineSpansPoint(verts[i], verts[i+1], p))
		//	continue;
		int orientation = lineMath::orientation(p, verts[i], verts[i+1]);
		if (orientation == 0)
			continue;
		double angle = lineMath::vectorAngle(p, verts[i], verts[i+1]);
		bool inside = (orientation > 0) == polyCW;
		winding += (inside ? +1 : -1) * angle;
	}
	if (out_winding)
		*out_winding = winding;
	return winding > 0.1;
}
