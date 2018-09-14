#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <cmath>

struct Transform {
	float scale = 1;	// real-to-virtual pixel ratio
	float transX = 0;	// virtual pixel offset
	float transY = 0;	// virtual pixel offset
};

struct ScreenPoint;

struct WorldPoint {
	float x = 0;
	float y = 0;

	ScreenPoint toScreen(Transform const& tr);

	float distanceTo(WorldPoint const& p) {
		float xdif = x - p.x;
		float ydif = y - p.y;
		return sqrt(xdif*xdif + ydif*ydif);
	}

	bool operator==(WorldPoint const& p) const {
		return x==p.x && y==p.y;
	}
};

struct ScreenPoint {
	int x=0;
	int y=0;

	WorldPoint toWorld(Transform const& tr);
};

struct GridPoint {
	int x=0;			// x coordinate on grid
	int y=0;			// y coordinate on grid
	float distance=0;	// distance from actual pixel to the grid location, relative to grid size [0.0 .. sqrt(2)/2]

	GridPoint() = default;
	GridPoint(int x, int y) : x(x), y(y), distance(0) {}
	GridPoint(int x, int y, float dist) : x(x), y(y), distance(dist) {}
};

#endif //__TRANSFORM_H__
