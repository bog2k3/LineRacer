#include "transform.h"
#include <cmath>
#include <algorithm>

ScreenPoint WorldPoint::toScreen(Transform const& tr) {
	int sx = (x + tr.transX) * tr.scale;
	int sy = (y + tr.transY) * tr.scale;
	return {sx, sy};
}

WorldPoint ScreenPoint::toWorld(Transform const& tr) {
	float wx = x / tr.scale - tr.transX;
	float wy = y / tr.scale - tr.transY;
	return {wx, wy};
}

std::pair<int, int> Arrow::direction() const {
	int x = to.x - from.x;
	int y = to.y - from.y;
	return {x, y};
}

int Arrow::length() const {
	auto dir = direction();
	int dx = abs(dir.first);
	int dy = abs(dir.second);
	return std::max(dx, dy);
}
