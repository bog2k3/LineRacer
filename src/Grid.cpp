#include "Grid.h"
#include "transform.h"

#include <SDL2/SDL_render.h>

Grid::Grid(int size, int winW, int winH) : size_(size), winW_(winW), winH_(winH) {
}

Grid::~Grid() {
}

void Grid::render(SDL_Renderer *r, Transform const& t) {
	SDL_SetRenderDrawColor(r, 190, 210, 255, SDL_ALPHA_OPAQUE);

	float squareSz = size_ * t.scale;
	int nSqX = ceil(winW_ / squareSz);
	int nSqY = ceil(winH_ / squareSz);
	float offsX = fmodf(t.transX, size_);
	if (offsX < 0)
		offsX += size_;
	offsX *= t.scale;
	float offsY = fmodf(t.transY, size_);
	if (offsY < 0)
		offsY += size_;
	offsY *= t.scale;

	// draw horizontal lines
	for (int i=0; i<nSqY; i++) {
		float y = offsY + i * squareSz;
		SDL_RenderDrawLine(r, 0, y, winW_, y);
	}

	// draw vertical lines
	for (int i=0; i<nSqX; i++) {
		float x = offsX + i * squareSz;
		SDL_RenderDrawLine(r, x, 0, x, winH_);
	}
}

ScreenPoint Grid::gridToScreen(GridPoint p, Transform const& t) {
	int sX = (p.x * size_ + t.transX) * t.scale;
	int sY = (p.y * size_ + t.transY) * t.scale;
	return {sX, sY};
}

GridPoint Grid::screenToGrid(ScreenPoint p, Transform const& t) {
	float gX = (p.x / t.scale - t.transX) / size_;
	float gY = (p.y / t.scale - t.transY) / size_;
	GridPoint ret {(int)gX, (int)gY, 0};
	gX = gX - (int)gX;
	if (abs(gX) > 0.5) {
		ret.x += gX > 0 ? 1 : -1;
		gX = 1.f - abs(gX);
	}
	gY = gY - (int)gY;
	if (abs(gY) > 0.5) {
		ret.y += gY > 0 ? 1 : -1;
		gY = 1.f - abs(gY);
	}
	ret.distance = sqrtf(gX*gX + gY*gY);
	return ret;
}
