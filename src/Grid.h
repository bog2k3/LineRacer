#ifndef __GRID_H__
#define __GRID_H__

#include "transform.h"

struct SDL_Renderer;

class Grid {
public:
	Grid(int size, int winW, int winH);
	~Grid();

	void setTransform(Transform t) { tr_ = t; }
	Transform const& getTransform() const { return tr_; }
	void render(SDL_Renderer *r);

	int cellSize() { return size_; }
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
