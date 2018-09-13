#ifndef __LINE_MATH_H__
#define __LINE_MATH_H__

struct WorldPoint;

namespace lineMath {

// returns +1 if the triangle formed by p,q,r is clockwise,
// -1 if it's counter-clockwise
// and zero if the points are collinear
int orientation(WorldPoint const& p, WorldPoint const& q, WorldPoint const& r);

// returns the sum of orientations of all angles in the polygon
// +1 for each clockwise corner, -1 for each counter-clockwise corner
// assumes a well-behaved (no self-intersections) and closed polygon (last vertex is assumed to connect back to first vertex)
int clockwiseness(const WorldPoint* points, unsigned n);

// returns true if line segments p1a->p1b intersects segment p2a->p2b
bool segmentIntersect(WorldPoint const& p1a, WorldPoint const& p1b, WorldPoint const& p2a, WorldPoint const& p2b);

} //namespace

#endif //__LINE_MATH_H__
