#ifndef __GRID_H__
#define __GRID_H__

struct SDL_Renderer;
struct Transform;

struct GridPoint {
	int x=0;			// x coordinate on grid
	int y=0;			// y coordinate on grid
	float distance=0;	// distance from actual pixel to the grid location, relative to grid size [0.0 .. sqrt(2)/2]

	GridPoint() = default;
	GridPoint(int x, int y) : x(x), y(y), distance(0) {}
	GridPoint(int x, int y, float dist) : x(x), y(y), distance(dist) {}
};

struct ScreenPoint {
	int x=0;
	int y=0;
};

class Grid {
public:
	Grid(int size, int winW, int winH);
	~Grid();

	void render(SDL_Renderer *r, Transform const& t);

	ScreenPoint gridToScreen(GridPoint p, Transform const& t);
	GridPoint screenToGrid(ScreenPoint p, Transform const& t);

private:
	int size_;
	int winW_, winH_;
};

#endif // __GRID_H__
