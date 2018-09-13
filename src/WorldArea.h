#ifndef __WORLD_AREA_H__
#define __WORLD_AREA_H__

#include "Grid.h"

struct WorldPoint;
struct SDL_Renderer;

class WorldArea {
public:
	WorldArea(Grid *g, GridPoint topLeft, GridPoint bottomRight);
	~WorldArea();

	void render(SDL_Renderer* r);

	bool containsPoint(WorldPoint const& wp) const;

	GridPoint topLeft() const { return topLeft_; }
	GridPoint bottomRight() const { return bottomRight_; }

private:
	Grid* grid_;
	GridPoint topLeft_;
	GridPoint bottomRight_;
};

#endif //__WORLD_AREA_H__
