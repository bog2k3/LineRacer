#include "transform.h"

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
