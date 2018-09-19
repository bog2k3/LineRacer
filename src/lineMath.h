#ifndef __LINE_MATH_H__
#define __LINE_MATH_H__

#include <cmath>

struct WorldPoint;

namespace lineMath {

enum IntersectionResult {
	INTERSECT_NONE = 0,	// segments don't intersect
	INTERSECT_MIDDLE,	// segments intersect somewhere in the middle
	INTERSECT_ENDPOINT1, // first segment's first endpoint lies on the other segment
	INTERSECT_ENDPOINT2, // first segment's second endpoint lies on the other segment
	INTERSECT_ENDPOINT3, // second segment's first endpoint lies on the other segment
	INTERSECT_ENDPOINT4, // second segment's second endpoint lies on the other segment
	INTERSECT_OVERLAP	// the segments overlap in many points
};

// returns +1 if the triangle formed by p,q,r is clockwise,
// -1 if it's counter-clockwise
// and zero if the points are collinear
int orientation(WorldPoint const& p, WorldPoint const& q, WorldPoint const& r);

// returns the sum of orientations of all angles in the polygon
// +1 for each clockwise corner, -1 for each counter-clockwise corner
// assumes a well-behaved (no self-intersections) and closed polygon (last vertex is assumed to connect back to first vertex)
int clockwiseness(const WorldPoint* points, unsigned n);

// returns the small angle between two vectors defined by: origin->A, origin->B;
double vectorAngle(WorldPoint const& origin, WorldPoint const& A, WorldPoint const& B);

// checks if line segment p1a->p1b intersects segment p2a->p2b
IntersectionResult segmentIntersect(WorldPoint const& p1a, WorldPoint const& p1b, WorldPoint const& p2a, WorldPoint const& p2b);

// returns the intersection point of two segments. This function assumes the segments DO intersect, so you need to check that before
WorldPoint intersectionPoint(WorldPoint const& p1a, WorldPoint const& p1b, WorldPoint const& p2a, WorldPoint const& p2b);

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(WorldPoint const& p, WorldPoint const& q, WorldPoint const& r);

// returns the direction from (0,0) to point p, in radians, CCW direction
template <class Point>
float pointDirection(Point p) {
	// normalize:
	float _1_len = 1.f / sqrtf(p.x*p.x + p.y*p.y);
	float x = p.x * _1_len;
	float y = p.y * _1_len;
	if (x >= 0)
		return asinf(y);
	else {
		auto s = y >= 0 ? 1 : -1;
		return -asinf(y) + M_PI*s;
	}
}

} //namespace

#endif //__LINE_MATH_H__
