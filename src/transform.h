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
};

struct ScreenPoint {
	int x=0;
	int y=0;

	WorldPoint toWorld(Transform const& tr);
};

#endif //__TRANSFORM_H__
