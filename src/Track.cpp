#include "Track.h"
#include "Grid.h"
#include "transform.h"
#include "WorldArea.h"
#include "color.h"
#include "lineMath.h"
#include "Painter.h"

#include <boglfw/renderOpenGL/Shape2D.h>

#include <cassert>

static const int PARTITION_CELL_SPAN = 5;

Track::Track(Grid* grid, WorldArea* warea, float resolution)
	: grid_(grid), worldArea_(warea), partition_(*this, PARTITION_CELL_SPAN), resolution_(resolution)
{
}

Track::~Track() {
}

void Track::draw(Viewport*) {
	static std::vector<glm::vec2> vPoints;
	vPoints.reserve(std::max(polyVertex_[0].size(), polyVertex_[1].size()));
	for (int i=0; i<2; i++) {
		if (polyVertex_[i].size() < 2)
			continue;
		vPoints.clear();
		for (unsigned j=0; j<polyVertex_[i].size(); j++) {
			ScreenPoint sp = polyVertex_[i][j].toScreen(grid_->getTransform());
			vPoints.push_back(sp);
		}
		Shape2D::get()->drawLineStrip(vPoints.data(), vPoints.size(), 0, Colors::TRACK);
	}
	if (designMode_ && designStep_ == TrackDesignStep::DRAW && polyVertex_[currentPolyIdx_].size() > 0) {
		ScreenPoint p1 = polyVertex_[currentPolyIdx_].back().toScreen(grid_->getTransform());
		ScreenPoint p2 = floatingVertex_.toScreen(grid_->getTransform());
		Shape2D::get()->drawLine(p1, p2, 0, Colors::TRACK_TRANSP);

		// if snapping close, draw the snap anchor
		if (polyVertex_[currentPolyIdx_].size() > 2 && floatingVertex_ == polyVertex_[currentPolyIdx_][0]) {
			const int snapAnchorRadius = 3;
			ScreenPoint anchor = floatingVertex_.toScreen(grid_->getTransform()) - ScreenPoint{snapAnchorRadius, snapAnchorRadius};
			Shape2D::get()->drawRectangleFilled(anchor, 0, {2*snapAnchorRadius+1, 2*snapAnchorRadius+1}, Colors::TRACK_SNAP);
		}
	}
	if (startLine_.isValid) {
		auto s1 = startLine_.p1.toScreen(grid_->getTransform());
		auto s2 = startLine_.p2.toScreen(grid_->getTransform());
		Shape2D::get()->drawLine(s1, s2, 0, Colors::STARTLINE);
		for (auto &spos : startLine_.startPositions) {
			auto p1 = grid_->gridToScreen(spos.position);
			auto p2 = grid_->gridToScreen({spos.position.x + spos.direction.first, spos.position.y + spos.direction.second});
			Painter::paintArrow(p1, p2, 10 * grid_->getTransform().scale, M_PI/6, Colors::STARTLINE);
		}
	}

#if 0 || DEBUG_CODE_TO_TEST_POINT_INSIDE_POLYGON
	static std::vector<std::pair<WorldPoint, bool>> debugPoints;
	static int sx = 0;
	static int sy = 0;
	if (designMode_ && currentPolyIdx_ == 1) {
		WorldPoint tl = grid_->gridToWorld(worldArea_->topLeft());
		WorldPoint br = grid_->gridToWorld(worldArea_->bottomRight());
		while (sy+tl.y < br.y) {
			while (sx+tl.x < br.x) {
				WorldPoint wp{tl.x + sx, tl.y + sy};
				bool green = pointInsidePolygon(wp, 0);
				debugPoints.push_back({wp, green});
				sx+=2;
			}
			sy+=2;
			sx = 0;
		}
		/*if (debugPoints.size() < 10000) {
			WorldPoint tl = grid_->gridToWorld(worldArea_->topLeft());
			WorldPoint br = grid_->gridToWorld(worldArea_->bottomRight());
			WorldPoint wp{rand() / (float)RAND_MAX * (br.x-tl.x) + tl.x, rand() / (float)RAND_MAX * (br.y-tl.y) + tl.y};
			bool green = pointInsidePolygon(wp, 0) && !pointInsidePolygon(wp, 1);
			debugPoints.push_back({wp, green});
		}*/
		//SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
		for (auto &p : debugPoints) {
			if (p.second)
				Colors::GREEN_TR.set(r);
			else
				Colors::RED_TR.set(r);
			SDL_Rect rc{p.first.x, p.first.y, 2, 2};
			SDL_RenderDrawRect(r, &rc);
		}
		//SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
	} else {
		debugPoints.clear();
		sx = sy = 0;
	}
#endif

#if 0 || DEBUG_CODE_TO_TEST_PARTITIONING
	for (int i=0; i<partition_.cells.size(); i++)
		for (int j=0; j<partition_.cells[i].size(); j++) {
			GridPoint tl {j*partition_.relativeCellSize_, i*partition_.relativeCellSize_};
			tl.x += worldArea_->topLeft().x;
			tl.y += worldArea_->topLeft().y;
			GridPoint br {(j+1)*partition_.relativeCellSize_, (i+1)*partition_.relativeCellSize_};
			br.x += worldArea_->topLeft().x;
			br.y += worldArea_->topLeft().y;
			ScreenPoint stl = grid_->gridToScreen(tl);
			ScreenPoint sbr = grid_->gridToScreen(br);
			SDL_Rect rc { stl.x, stl.y, sbr.x-stl.x-1, sbr.y-stl.y-1 };
			if (partition_.cells[i][j].size())
				Colors::GREEN_TR.set(r);
			else
				Colors::RED_TR.set(r);
			SDL_RenderFillRect(r, &rc);
		}
#endif

#if 0 || DEBUG_CODE_TO_TEST_PARTITION_DENSITY
	auto verts = partition_.getVerticesInArea(floatingVertex_, floatingVertex_);
	Colors::RED.set(r);
	for (auto &v : verts) {
		if (v.second + 1 < polyVertex_[v.first].size()) {
			ScreenPoint p1 = polyVertex_[v.first][v.second].toScreen(grid_->getTransform());
			ScreenPoint p2 = polyVertex_[v.first][v.second+1].toScreen(grid_->getTransform());
			SDL_RenderDrawLine(r, p1.x, p1.y, p2.x, p2.y);
		}
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
	// compute start line orientation:
	WorldPoint &outer = intersections[imin].polyId == 0 ? startLine_.p1 : startLine_.p2;	// intersection point on outer polygon
	WorldPoint &inner = intersections[imin].polyId == 0 ? startLine_.p2 : startLine_.p1;	// intersection point on inner polygon
	startLine_.orientation = lineMath::orientation(inner, outer, floatingVertex_);
	// compute valid starting arrows:
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
				polyOrientation_[currentPolyIdx_] = lineMath::clockwiseness(polyVertex_[currentPolyIdx_].data(), polyVertex_[currentPolyIdx_].size()-1) > 0 ? +1 : -1;
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

unsigned Track::intersectionsCount(GridPoint const& p1, GridPoint const& p2) const {
	WorldPoint wp1 = grid_->gridToWorld(p1);
	WorldPoint wp2 = grid_->gridToWorld(p2);
	WorldPoint topLeft {std::min(wp1.x, wp2.x), std::min(wp1.y, wp2.y)};
	WorldPoint bottomRight {std::max(wp1.x, wp2.x), std::max(wp1.y, wp2.y)};
	auto vertices = partition_.getVerticesInArea(topLeft, bottomRight);
	unsigned count = 0;
	for (auto &v : vertices) {
		int polyIdx = v.first;
		int vIdx = v.second;
		if ((unsigned)vIdx+1 == polyVertex_[polyIdx].size())
			continue;	// this was the last vertex, we skip it
		// we check forward, from this vertex to the next
		const WorldPoint &v1 = polyVertex_[polyIdx][vIdx];
		const WorldPoint &v2 = polyVertex_[polyIdx][vIdx+1];
		lineMath::IntersectionResult res = lineMath::segmentIntersect(v1, v2, wp1, wp2);
		if (res == lineMath::INTERSECT_NONE || res == lineMath::INTERSECT_OVERLAP)
			continue;
		bool consider = false;
		if (res == lineMath::INTERSECT_MIDDLE)
			consider = true;
		else if (res == lineMath::INTERSECT_ENDPOINT1)
			// v1 is on wp1->wp2 we count it only if the line is going downward
			consider = v2.y > v1.y;
		else if (res == lineMath::INTERSECT_ENDPOINT2)
			// v2 is on wp1->wp2, we count it only if the line is going upward
			consider = v2.y < v1.y;

		if (consider)
			count++;
	}
	return count;
}

bool Track::pointInsidePolygon(WorldPoint const& p, int polyIndex) const {
	assert(polyIndex >= 0 && polyIndex <= 1);
	if (polyVertex_[polyIndex].size() < 3)
		return false;
	if (!worldArea_->containsPoint(p))
		return false;
	int polyOrientation = polyOrientation_[polyIndex];
	// draw an imaginary horizontal line from the left limit of worldArea through point p
	// then see how many edges from the test polygon it intersects
	// odd means point is inside, even means it's outside
	WorldPoint start {grid_->gridToWorld(worldArea_->topLeft()).x-1, p.y};
	auto verts = partition_.getVerticesInArea({start.x, p.y-1}, {p.x+1, p.y + 1});
	int wn = 0;
	for (auto &v : verts) {
		if (v.first != polyIndex)
			continue;
		// test edge before vertex
		if (v.second > 0) {
			const WorldPoint &v1 = polyVertex_[polyIndex][v.second-1];
			const WorldPoint &v2 = polyVertex_[polyIndex][v.second];
			if (v1.x >= p.x && v2.x >= p.x)
				continue;
			lineMath::IntersectionResult res = lineMath::segmentIntersect(v1, v2, start, p);
			if (res == lineMath::INTERSECT_NONE || res == lineMath::INTERSECT_OVERLAP)
				continue;
			bool count = false;
			if (res == lineMath::INTERSECT_MIDDLE)
				count = true;
			else if (res == lineMath::INTERSECT_ENDPOINT1)
				// v1 is on the horizontal line, we count it only if the line is going downward
				count = v2.y > v1.y;
			else if (res == lineMath::INTERSECT_ENDPOINT2)
				// v2 is on the horizontal line, we count it only if the line is going upward
				count = v2.y < v1.y;

			if (!count)
				continue;
			int side = lineMath::orientation(v1, v2, p);
			wn += side == polyOrientation ? +1 : -1;
		}
	}
	return wn != 0;
}

std::pair<int, float> Track::computeCrossingIndex(GridPoint const& p1, GridPoint const& p2) {
	WorldPoint wp1 = grid_->gridToWorld(p1);
	WorldPoint wp2 = grid_->gridToWorld(p2);
	WorldPoint topLeft {std::min(wp1.x, wp2.x), std::min(wp1.y, wp2.y)};
	WorldPoint bottomRight {std::max(wp1.x, wp2.x), std::max(wp1.y, wp2.y)};
	auto verts = partition_.getVerticesInArea(topLeft, bottomRight);
	for (auto &v : verts) {
		int pIndex = v.first;
		int vIndex = v.second;
		if ((unsigned)vIndex + 1 == polyVertex_[pIndex].size())
			continue;
		WorldPoint &sp1 = polyVertex_[pIndex][vIndex];
		WorldPoint &sp2 = polyVertex_[pIndex][vIndex + 1];
		if (lineMath::segmentIntersect(sp1, sp2, wp1, wp2)) {
			auto crossPoint = lineMath::intersectionPoint(sp1, sp2, wp1, wp2);
			float segLength = lineMath::distance(sp1, sp2);
			float crossLength = lineMath::distance(sp1, crossPoint);
			assert(crossLength <= segLength);
			return {pIndex, vIndex + crossLength / segLength};
		}
	}
	return {-1, -1};
}

int Track::polyDirection(unsigned polyIndex) const {
	assert(polyIndex <= 1);
	if (!startLine_.isValid)
		return 0;
	return polyOrientation_[polyIndex] == startLine_.orientation ? +1 : -1;
}

int Track::checkStartLineCross(GridPoint const& from, GridPoint const& to, bool extended, WorldPoint* out_Point) const {
	WorldPoint fw = grid_->gridToWorld(from);
	WorldPoint tw = grid_->gridToWorld(to);
	lineMath::IntersectionResult res;
	if (extended)
		res = lineMath::segmentIntersectLine(fw, tw, startLine_.p1, startLine_.p2);
	else
		res = lineMath::segmentIntersect(fw, tw, startLine_.p1, startLine_.p2);
	if (res == lineMath::INTERSECT_NONE || res == lineMath::INTERSECT_OVERLAP || res == lineMath::INTERSECT_ENDPOINT1)
		return 0;
	if (out_Point) {
		*out_Point = lineMath::intersectionPoint(fw, tw, startLine_.p1, startLine_.p2, extended);
	}
	int dot = (to.x - from.x) * startLine_.startPositions[0].direction.first +
			(to.y - from.y) * startLine_.startPositions[0].direction.second;
	return dot > 0 ? +1 : -1;
}
