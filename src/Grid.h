#ifndef __GRID_H__
#define __GRID_H__

#include "transform.h"

struct SDL_Renderer;

struct GridPoint {
	int x=0;			// x coordinate on grid
	int y=0;			// y coordinate on grid
	float distance=0;	// distance from actual pixel to the grid location, relative to grid size [0.0 .. sqrt(2)/2]

	GridPoint() = default;
	GridPoint(int x, int y) : x(x), y(y), distance(0) {}
	GridPoint(int x, int y, float dist) : x(x), y(y), distance(dist) {}
};

class Grid {
public:
	Grid(int size, int winW, int winH);
	~Grid();

	void setTransform(Transform t) { tr_ = t; }
	Transform const& getTransform() const { return tr_; }
	void render(SDL_Renderer *r);

	int size() { return size_; }
	ScreenPoint gridToScreen(GridPoint p) const;
	WorldPoint gridToWorld(GridPoint p) const;
	GridPoint screenToGrid(ScreenPoint p) const;
	GridPoint worldToGrid(WorldPoint p) const;

private:
	int size_;
	int winW_, winH_;
	Transform tr_;
};

#endif // __GRID_H__
