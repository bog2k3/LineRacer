#ifndef __WORLD_AREA_H__
#define __WORLD_AREA_H__

struct GridPoint;
struct WorldPoint;
struct SDL_Renderer;
class Grid;

class WorldArea {
public:
	WorldArea(Grid *g, GridPoint topLeft, GridPoint bottomRight);
	~WorldArea();

	void render(SDL_Renderer* r);

	bool containsPoint(WorldPoint const& wp) const;

private:
	Grid* grid_;
	int gX_, gY_;
	int gW_, gH_;
};

#endif //__WORLD_AREA_H__
