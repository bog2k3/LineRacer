#include "Grid.h"
#include "transform.h"
#include "color.h"

#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/math/math3D.h>

Grid::Grid(int size, int winW, int winH) : size_(size), winW_(winW), winH_(winH) {
}

Grid::~Grid() {
}

void Grid::draw(Viewport*) {
	float squareSz = size_ * tr_.scale;
	int nSqX = ceil(winW_ / squareSz);
	int nSqY = ceil(winH_ / squareSz);
	float offsX = fmodf(tr_.transX, size_);
	if (offsX < 0)
		offsX += size_;
	offsX *= tr_.scale;
	float offsY = fmodf(tr_.transY, size_);
	if (offsY < 0)
		offsY += size_;
	offsY *= tr_.scale;

	// draw horizontal lines
	for (int i=0; i<nSqY; i++) {
		float y = offsY + i * squareSz;
		Shape2D::get()->drawLine({0, y}, {winW_, y}, 0, Colors::GRID);
	}

	// draw vertical lines
	for (int i=0; i<nSqX; i++) {
		float x = offsX + i * squareSz;
		Shape2D::get()->drawLine({x, 0}, {x, winH_}, 0, Colors::GRID);
	}
}

ScreenPoint Grid::gridToScreen(GridPoint p) const {
	int sX = (p.x * size_ + tr_.transX) * tr_.scale;
	int sY = (p.y * size_ + tr_.transY) * tr_.scale;
	return {sX, sY};
}

WorldPoint Grid::gridToWorld(GridPoint p) const {
	return {(float)p.x * size_, (float)p.y * size_};
}

GridPoint Grid::screenToGrid(ScreenPoint p) const {
	return worldToGrid(p.toWorld(tr_));
}

GridPoint Grid::worldToGrid(WorldPoint wp) const {
	float gX = wp.x / size_;
	float gY = wp.y / size_;
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
