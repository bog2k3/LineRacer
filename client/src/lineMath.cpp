#include "lineMath.h"
#include "transform.h"

#include <algorithm>
#include <cmath>
#include <cassert>

namespace lineMath {

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(WorldPoint const& p, WorldPoint const& q, WorldPoint const& r)
{
	if (orientation(p, q, r) != 0)
		return false;
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
    double val = ((double)q.y - (double)p.y) * ((double)r.x - (double)q.x) -
				((double)q.x - (double)p.x) * ((double)r.y - (double)q.y);

    if (abs(val) < 0.000001) return 0;  // colinear

    return val > 0 ? +1 : -1; // clock or counterclock wise
}

int clockwiseness(const WorldPoint* points, unsigned n) {
	if (n < 3)
		return 0;
	int ret = 0;
	for (unsigned i=1; i<=n; i++)
		ret += orientation(points[(i-1)%n], points[i%n], points[(i+1)%n]);
	return ret;
}

double vectorAngle(WorldPoint const& origin, WorldPoint const& A, WorldPoint const& B) {
	double v1x = (double)A.x - origin.x;
	double v1y = (double)A.y - origin.y;
	double v2x = (double)B.x - origin.x;
	double v2y = (double)B.y - origin.y;
	// normalize vectors
	double v1l_inv = 1.f / sqrt(v1x*v1x + v1y*v1y);
	v1x *= v1l_inv; v1y *= v1l_inv;
	float v2l_inv = 1.f / sqrt(v2x*v2x + v2y*v2y);
	v2x *= v2l_inv; v2y *= v2l_inv;
	double angle = acos(v1x*v2x + v1y*v2y);	// this is the small angle
	return angle;
}

IntersectionResult segmentIntersect(WorldPoint const& p1a, WorldPoint const& p1b, WorldPoint const& p2a, WorldPoint const& p2b) {
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1a, p1b, p2a);
    int o2 = orientation(p1a, p1b, p2b);
    int o3 = orientation(p2a, p2b, p1a);
    int o4 = orientation(p2a, p2b, p1b);

    // General case
    if (o1 != o2 && o3 != o4 && o1*o2*o3*o4 != 0)
        return INTERSECT_MIDDLE;

    // Special Cases
    // p1a, p1b and p2a are colinear and p2a lies on segment p1q1
    if (o1 == 0 && onSegment(p1a, p2a, p1b)) {
		if (onSegment(p2a, p1a, p2b) || onSegment(p2a, p1b, p2b) || onSegment(p1a, p2b, p1b))
			return INTERSECT_OVERLAP;
		else
			return INTERSECT_ENDPOINT3;
	}

    // p1a, p1b and p2b are colinear and p2b lies on segment p1q1
    if (o2 == 0 && onSegment(p1a, p2b, p1b)) {
		if (onSegment(p2a, p1a, p2b) || onSegment(p2a, p1b, p2b) || onSegment(p1a, p2a, p1b))
			return INTERSECT_OVERLAP;
		else
			return INTERSECT_ENDPOINT4;
	}

    // p2a, p2b and p1a are colinear and p1a lies on segment p2q2
    if (o3 == 0 && onSegment(p2a, p1a, p2b)) {
		if ( onSegment(p1a, p2a, p1b) || onSegment(p1a, p2b, p1b) || onSegment(p2a, p1b, p2b))
			return INTERSECT_OVERLAP;
		else
			return INTERSECT_ENDPOINT1;
	}

     // p2a, p2b and p1b are colinear and p1b lies on segment p2q2
    if (o4 == 0 && onSegment(p2a, p1b, p2b)) {
		if (onSegment(p1a, p2a, p1b) || onSegment(p1a, p2b, p1b) || onSegment(p2a, p1a, p2b))
			return INTERSECT_OVERLAP;
		else
			return INTERSECT_ENDPOINT2;
	}

    return INTERSECT_NONE; // Doesn't fall in any of the above cases
}

IntersectionResult segmentIntersectLine(WorldPoint const& s1, WorldPoint const& s2, WorldPoint const& l1, WorldPoint const& l2) {
	int o1 = orientation(l1, l2, s1);
	int o2 = orientation(l1, l2, s2);

	if (o1 == 0 && o2 == 0)
		return INTERSECT_OVERLAP;
	if (o1 == o2)
		return INTERSECT_NONE;
	if (o1 == 0)
		return INTERSECT_ENDPOINT1;
	if (o2 == 0)
		return INTERSECT_ENDPOINT2;
	return INTERSECT_MIDDLE;
}

// this is ugly as hell, but does the job
WorldPoint intersectionPoint(WorldPoint const& p, WorldPoint const& p2, WorldPoint const& q, WorldPoint const& q2, bool extendSecond) {
	auto cross = [](float ax, float ay, float bx, float by) { return ax * by - ay * bx; };
	auto isZero = [](float x) { return abs(x) < 1e-10f; };
	auto dot = [](float ax, float ay, float bx, float by) { return ax*bx + ay*by; };

    float r[] = {p2.x - p.x, p2.y - p.y};
    float s[] = {q2.x - q.x, q2.y - q.y};
    float rxs = cross(r[0], r[1], s[0], s[1]);
    float qpxr = cross(q.x - p.x, q.y - p.y, r[0], r[1]);

    // If r x s = 0 and (q - p) x r = 0, then the two lines are collinear.
    if (isZero(rxs) && isZero(qpxr)) {
        // 1. If either  0 <= (q - p) * r <= r * r or 0 <= (p - q) * s <= * s
        // then the two lines are overlapping,
		float qpdotr = dot(q.x - p.x, q.y - p.y, r[0], r[1]);
		if (0 <= qpdotr && qpdotr <= dot(r[0], r[1], r[0], r[1])) {
			return q;
		}
		float pqdots = dot(p.x - q.x, p.y - q.y, s[0], s[1]);
		if (0 <= pqdots && pqdots <= dot(s[0], s[1], s[0], s[1])) {
			return p;
		}
		if (extendSecond)
			return p2;
		else {
			assert(false && "you didn't check the segments for intersection before calling this, did you?");
			return {0, 0};
		}
    }

    // t = (q - p) x s / (r x s)
    float t = cross(q.x - p.x, q.y - p.y, s[0], s[1]) / rxs;

    // u = (q - p) x r / (r x s)
    float u = cross(q.x - p.x, q.y - p.y, r[0], r[1]) / rxs;

    // 4. If r x s != 0 and 0 <= t <= 1 and 0 <= u <= 1
    // the two line segments meet at the point p + t r = q + u s.
	bool tWithinEndpoints = 0 <= t && t <= 1;
	bool uWithinEndpoints = 0 <= u && u <= 1;
    if (!isZero(rxs) && tWithinEndpoints && (extendSecond || uWithinEndpoints)) {
        // We can calculate the intersection point using either t or u.
        return {p.x + t*r[0], p.y + t*r[1]};
    }
	// if we got here, the lines are parallel and there's a non-zero perpendicular distance between them
	assert(false && "you didn't check the segments for intersection before calling this, did you?");
	return {0, 0};
}

} // namespace
