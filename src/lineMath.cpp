#include "lineMath.h"
#include "transform.h"

#include <algorithm>

namespace lineMath {

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(WorldPoint const& p, WorldPoint const& q, WorldPoint const& r)
{
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
        q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
       return true;

    return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// +1 --> Clockwise
// -1 --> Counterclockwise
int orientation(WorldPoint const& p, WorldPoint const& q, WorldPoint const& r) {
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
    // for details of below formula.
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;  // colinear

    return (val > 0)? +1: -1; // clock or counterclock wise
}

int clockwiseness(const WorldPoint* points, unsigned n) {
	if (n < 3)
		return 0;
	int ret = 0;
	for (unsigned i=1; i<=n; i++)
		ret += orientation(points[(i-1)%n], points[i%n], points[(i+1)%n]);
	return ret;
}

bool segmentIntersect(WorldPoint const& p1a, WorldPoint const& p1b, WorldPoint const& p2a, WorldPoint const& p2b) {
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1a, p1b, p2a);
    int o2 = orientation(p1a, p1b, p2b);
    int o3 = orientation(p2a, p2b, p1a);
    int o4 = orientation(p2a, p2b, p1b);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1a, p1b and p2a are colinear and p2a lies on segment p1q1
    if (o1 == 0 && onSegment(p1a, p2a, p1b)) return true;

    // p1a, p1b and p2b are colinear and p2b lies on segment p1q1
    if (o2 == 0 && onSegment(p1a, p2b, p1b)) return true;

    // p2a, p2b and p1a are colinear and p1a lies on segment p2q2
    if (o3 == 0 && onSegment(p2a, p1a, p2b)) return true;

     // p2a, p2b and p1b are colinear and p1b lies on segment p2q2
    if (o4 == 0 && onSegment(p2a, p1b, p2b)) return true;

    return false; // Doesn't fall in any of the above cases
}

/*
IntersectResult segmentIntersect(WorldPoint const& p1a, WorldPoint const& p1b, WorldPoint const& p2a, WorldPoint const& p2b) {
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1a, p1b, p2a);
    int o2 = orientation(p1a, p1b, p2b);
    int o3 = orientation(p2a, p2b, p1a);
    int o4 = orientation(p2a, p2b, p1b);

    // General case
    if (o1 != o2 && o3 != o4) {
		if (o1 * o2 != 0 && o3*o4 != 0)
        	return IntersectResult::MIDDLE;
		else
			return IntersectResult::ENDPOINT;
	}

    // Special Cases
    // p1a, p1b and p2a are colinear and p2a lies on segment p1q1
    if (o1 == 0 && onSegment(p1a, p2a, p1b)) {
		if (p1b == p1a || p1b == p2a)
			return IntersectResult::ENDPOINT;
		else
			return IntersectResult::MIDDLE;
	}

    // p1a, p1b and p2b are colinear and p2b lies on segment p1q1
    if (o2 == 0 && onSegment(p1a, p2b, p1b)) {
		if (p1a == p1b || p1a == p2b)
			return IntersectResult::ENDPOINT;
		else
			return IntersectResult::MIDDLE;
	}

    // p2a, p2b and p1a are colinear and p1a lies on segment p2q2
    if (o3 == 0 && onSegment(p2a, p1a, p2b)) {
		if (p2b == p1a || p2b == p2a)
			return IntersectResult::ENDPOINT;
		else
			return IntersectResult::MIDDLE;
	}

     // p2a, p2b and p1b are colinear and p1b lies on segment p2q2
    if (o4 == 0 && onSegment(p2a, p1b, p2b)) {
		if (p2a == p1b || p2a == p2b)
			return IntersectResult::ENDPOINT;
		else
			return IntersectResult::MIDDLE;
	}

    return IntersectResult::NONE; // Doesn't fall in any of the above cases
}
*/

} // namespace
