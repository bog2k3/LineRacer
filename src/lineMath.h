#ifndef __LINE_MATH_H__
#define __LINE_MATH_H__

struct WorldPoint;

namespace lineMath {

// returns positive if the triangle formed by p,q,r is clockwise,
// negative if it's counter-clockwise
// and zero if the points are collinear
int orientation(WorldPoint const& p, WorldPoint const& q, WorldPoint const& r);

// returns a value between [-2*PI..+2*PI] negative meanse counter-clockwise, positive means clockwise
// if all points are collinear, the return value will be zero
float clockwiseness(WorldPoint* points, unsigned n);

// returns true if line segments p1a->p1b intersects segment p2a->p2b
bool segmentIntersect(WorldPoint const& p1a, WorldPoint const& p1b, WorldPoint const& p2a, WorldPoint const& p2b);

} //namespace

#endif //__LINE_MATH_H__
