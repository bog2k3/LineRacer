#ifndef __TRACK_H__
#define __TRACK_H__

#include "transform.h"
#include "TrackPartition.h"

#include <vector>
#include <cassert>

class Grid;
class WorldArea;
struct GridPoint;
class Viewport;

enum class TrackDesignStep {
	DRAW,
	STARTLINE
};

struct StartPosition {
	GridPoint position;
	std::pair<int, int> direction;
};

class Track {
public:
	// resolution : this is relative to grid's cell size, a resolution of 2 means the track vertices are twice as dense as the grid
	Track(Grid* grid, WorldArea* warea, float resolution);
	~Track();

	void draw(Viewport*);

	void reset(); // clears the track
	void enableDesignMode(bool enable);
	void setDesignStep(TrackDesignStep step);
	bool isInDesignMode() const { return designMode_; }
	void pointerMoved(float x, float y);	// x and y are world-space coordinates
	void pointerTouch(bool on, float x, float y); // x and y are world-space coordinates

	bool isReady() const { return !designMode_ && startLine_.isValid; }	// returns true if the track is ready for the game

	std::vector<StartPosition> getStartPositions() const { return isReady() ? startLine_.startPositions : decltype(startLine_.startPositions){}; }

	// returns true if a line from grid point p1 to p2 intersects a track segment
	// out_point: optional parameter -> intersection point will be stored in it
	// out_polyIndex: optional -> polygon index that was intersected will be stored in it
	bool intersectLine(GridPoint const& p1, GridPoint const& p2, WorldPoint* out_point=nullptr, int *out_polyIndex=nullptr) const;
	// returns the number of points in which the arrow intersects the track polygons
	unsigned intersectionsCount(GridPoint const& p1, GridPoint const& p2) const;
	// returns true if the point is inside the closed polygon
	bool pointInsidePolygon(WorldPoint const& p, int polyIndex) const;
	// crossing index indicates the point where the line [p1, p2] intersects a track contour;
	// the integral part represents the segment index, and the decimal part represents the position on that segment
	// first is contour index, second is the crossing index into that contour
	std::pair<int, float> computeCrossingIndex(GridPoint const& p1, GridPoint const& p2);
	// returns the "forward" vertex direction for a given polygon with respect to the startLine's direction;
	// if the startline points in the same direction as the polygon winding, will return +1, otherwise -1;
	// returns 0 if the request is not valid in the current state
	int polyDirection(unsigned polyIndex) const;
	// returns the length (in vertices) of the specified polygon (0: outer poly, 1: inner poly)
	unsigned polyLength(unsigned index) const { assert(index <= 1); return polyVertex_[index].size(); }
	// returns the [index] vertex from [poly] polygon
	WorldPoint polyVertex(unsigned poly, unsigned index) const { assert(poly<=1 && index < polyVertex_[poly].size()); return polyVertex_[poly][index]; }
	// checks if a line [from, to] crosses the start-line
	//		* checks segment if extended==false
	//		* checks infinite line if extended==true
	// returns +1 if the line crosses the start line from back to front
	// returns -1 if the line crosses the start line from front to back
	// returns 0 if they don't cross
	// out_Point: if not null, will be filled with the intersection point
	int checkStartLineCross(GridPoint const& from, GridPoint const& to, bool extended, WorldPoint* out_Point=nullptr) const;

	Grid* grid() const { return grid_; }
	WorldArea* worldArea() const { return worldArea_; }

private:
	friend class TrackPartition;

	Grid* grid_;
	WorldArea* worldArea_;
	TrackPartition partition_;
	float resolution_;
	bool designMode_ = false;
	bool pointerPressed_ = false;
	int currentPolyIdx_ = 0;
	WorldPoint floatingVertex_;
	TrackDesignStep designStep_ = TrackDesignStep::DRAW;

	struct StartLine {
		WorldPoint p1;
		WorldPoint p2;
		std::vector<StartPosition> startPositions;
		int orientation;
		bool isValid = false;
	} startLine_;

	// track consists of two closed polygons, one inner and one outer
	std::vector<WorldPoint> polyVertex_[2];
	int polyOrientation_[2];

	bool checkCloseSnap();
	bool validateVertex();
	void pushVertex();
	// returns true if a line from point p1 to p2 intersects a track segment
	bool intersectLine(WorldPoint const& p1, WorldPoint const& p2, bool skipLastSegment, WorldPoint* out_point=nullptr, int *out_polyIndex=nullptr) const;
	void updateStartLine();
	void saveStartLine();
};

#endif //__TRACK_H__
