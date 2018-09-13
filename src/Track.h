#ifndef __TRACK_H__
#define __TRACK_H__

#include "transform.h"
#include "TrackPartition.h"

#include <vector>

class Grid;
class WorldArea;
struct GridPoint;
struct SDL_Renderer;

class Track {
public:
	// resolution : this is relative to grid's cell size, a resolution of 2 means the track vertices are twice as dense as the grid
	Track(Grid* grid, WorldArea* warea, float resolution);
	~Track();

	void render(SDL_Renderer* r);

	void reset(); // clears the track
	void enableDesignMode(bool enable);
	bool isInDesignMode() const { return designMode_; }
	void pointerMoved(float x, float y);	// x and y are world-space coordinates
	void pointerTouch(bool on, float x, float y); // x and y are world-space coordinates

	// returns true if a line from grid point p1 to p2 intersects a track segment
	bool intersectLine(GridPoint const& p1, GridPoint const& p2) const;
	// returns true if the point is inside the closed polygon
	bool pointInsidePolygon(WorldPoint const& p, int polyIndex) const;

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

	// track consists of two closed polygons, one inner and one outer
	std::vector<WorldPoint> polyVertex_[2];

	bool checkCloseSnap();
	bool validateVertex();
	void pushVertex();
};

#endif //__TRACK_H__
