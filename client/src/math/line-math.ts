import { assert } from "../utils/assert";
import { IPoint, WorldPoint } from "./math";

export namespace lineMath {
	export enum IntersectionResult {
		/** segments don't intersect */
		INTERSECT_NONE,
		/** segments intersect somewhere in the middle */
		INTERSECT_MIDDLE,
		/** first segment's first endpoint lies on the other segment */
		INTERSECT_ENDPOINT1,
		/** first segment's second endpoint lies on the other segment */
		INTERSECT_ENDPOINT2,
		/** second segment's first endpoint lies on the other segment */
		INTERSECT_ENDPOINT3,
		/** second segment's second endpoint lies on the other segment */
		INTERSECT_ENDPOINT4,
		/** the segments overlap in many points */
		INTERSECT_OVERLAP,
	}

	/**
	 * @returns +1 if the triangle formed by p,q,r is clockwise,
	 * 			-1 if it's counter-clockwise,
	 * 			 0 if the points are collinear
	 */
	export function orientation(p: WorldPoint, q: WorldPoint, r: WorldPoint): number {
		// See https://www.geeksforgeeks.org/orientation-3-ordered-points/
		// for details of below formula.
		const val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

		if (Math.abs(val) < 0.000001) return 0; // colinear

		return val > 0 ? +1 : -1; // clock or counterclock wise
	}

	/**
	 * @returns the sum of orientations of all angles in the polygon
	 *		+1 for each clockwise corner, -1 for each counter-clockwise corner.
	 *		assumes a well-behaved (no self-intersections) and closed polygon (last vertex is assumed to connect back to first vertex)
	 */
	export function clockwiseness(points: WorldPoint[], n: number): number {
		if (n < 3) return 0;
		let ret = 0;
		for (let i = 1; i <= n; i++) ret += orientation(points[(i - 1) % n], points[i % n], points[(i + 1) % n]);
		return ret;
	}

	/**
	 * @returns the small angle between two vectors defined by: origin->A, origin->B;
	 */
	export function vectorAngle(origin: WorldPoint, A: WorldPoint, B: WorldPoint): number {
		let v1x = A.x - origin.x;
		let v1y = A.y - origin.y;
		let v2x = B.x - origin.x;
		let v2y = B.y - origin.y;
		// normalize vectors
		const v1l_inv = 1.0 / Math.sqrt(v1x * v1x + v1y * v1y);
		v1x *= v1l_inv;
		v1y *= v1l_inv;
		const v2l_inv = 1.0 / Math.sqrt(v2x * v2x + v2y * v2y);
		v2x *= v2l_inv;
		v2y *= v2l_inv;
		const angle = Math.acos(v1x * v2x + v1y * v2y); // this is the small angle
		return angle;
	}

	/**
	 * Checks if line segment p1a->p1b intersects segment p2a->p2b
	 */
	export function segmentIntersect(
		p1a: WorldPoint,
		p1b: WorldPoint,
		p2a: WorldPoint,
		p2b: WorldPoint,
	): IntersectionResult {
		// Find the four orientations needed for general and special cases
		const o1 = orientation(p1a, p1b, p2a);
		const o2 = orientation(p1a, p1b, p2b);
		const o3 = orientation(p2a, p2b, p1a);
		const o4 = orientation(p2a, p2b, p1b);

		// General case
		if (o1 != o2 && o3 != o4 && o1 * o2 * o3 * o4 != 0) return IntersectionResult.INTERSECT_MIDDLE;

		// Special Cases
		// p1a, p1b and p2a are colinear and p2a lies on segment p1q1
		if (o1 == 0 && onSegment(p1a, p2a, p1b)) {
			if (onSegment(p2a, p1a, p2b) || onSegment(p2a, p1b, p2b) || onSegment(p1a, p2b, p1b))
				return IntersectionResult.INTERSECT_OVERLAP;
			else return IntersectionResult.INTERSECT_ENDPOINT3;
		}

		// p1a, p1b and p2b are colinear and p2b lies on segment p1q1
		if (o2 == 0 && onSegment(p1a, p2b, p1b)) {
			if (onSegment(p2a, p1a, p2b) || onSegment(p2a, p1b, p2b) || onSegment(p1a, p2a, p1b))
				return IntersectionResult.INTERSECT_OVERLAP;
			else return IntersectionResult.INTERSECT_ENDPOINT4;
		}

		// p2a, p2b and p1a are colinear and p1a lies on segment p2q2
		if (o3 == 0 && onSegment(p2a, p1a, p2b)) {
			if (onSegment(p1a, p2a, p1b) || onSegment(p1a, p2b, p1b) || onSegment(p2a, p1b, p2b))
				return IntersectionResult.INTERSECT_OVERLAP;
			else return IntersectionResult.INTERSECT_ENDPOINT1;
		}

		// p2a, p2b and p1b are colinear and p1b lies on segment p2q2
		if (o4 == 0 && onSegment(p2a, p1b, p2b)) {
			if (onSegment(p1a, p2a, p1b) || onSegment(p1a, p2b, p1b) || onSegment(p2a, p1a, p2b))
				return IntersectionResult.INTERSECT_OVERLAP;
			else return IntersectionResult.INTERSECT_ENDPOINT2;
		}

		return IntersectionResult.INTERSECT_NONE; // Doesn't fall in any of the above cases
	}

	/**
	 * Checks if segment s1->s2 intersects infinite line l1->l2
	 */
	export function segmentIntersectLine(
		s1: WorldPoint,
		s2: WorldPoint,
		l1: WorldPoint,
		l2: WorldPoint,
	): IntersectionResult {
		const o1 = orientation(l1, l2, s1);
		const o2 = orientation(l1, l2, s2);

		if (o1 == 0 && o2 == 0) return IntersectionResult.INTERSECT_OVERLAP;
		if (o1 == o2) return IntersectionResult.INTERSECT_NONE;
		if (o1 == 0) return IntersectionResult.INTERSECT_ENDPOINT1;
		if (o2 == 0) return IntersectionResult.INTERSECT_ENDPOINT2;
		return IntersectionResult.INTERSECT_MIDDLE;
	}

	/**
	 * @returns the intersection point of two segments p->p2 & q->q2 (or one segment and one line). This function assumes the segments DO intersect, so you need to check that before
	 * if extendSecond=true then q->q2 is treated as an infinite line
	 */
	export function intersectionPoint(
		p: WorldPoint,
		p2: WorldPoint,
		q: WorldPoint,
		q2: WorldPoint,
		extendSecond = false,
	): WorldPoint {
		// this is ugly as hell, but does the job
		const cross = (ax, ay, bx, by) => ax * by - ay * bx;
		const isZero = (x) => Math.abs(x) < 1e-10;
		const dot = (ax, ay, bx, by) => ax * bx + ay * by;

		const r: number[] = [p2.x - p.x, p2.y - p.y];
		const s: number[] = [q2.x - q.x, q2.y - q.y];
		const rxs: number = cross(r[0], r[1], s[0], s[1]);
		const qpxr: number = cross(q.x - p.x, q.y - p.y, r[0], r[1]);

		// If r x s = 0 and (q - p) x r = 0, then the two lines are collinear.
		if (isZero(rxs) && isZero(qpxr)) {
			// 1. If either  0 <= (q - p) * r <= r * r or 0 <= (p - q) * s <= * s
			// then the two lines are overlapping,
			const qpdotr = dot(q.x - p.x, q.y - p.y, r[0], r[1]);
			if (0 <= qpdotr && qpdotr <= dot(r[0], r[1], r[0], r[1])) {
				return q;
			}
			const pqdots = dot(p.x - q.x, p.y - q.y, s[0], s[1]);
			if (0 <= pqdots && pqdots <= dot(s[0], s[1], s[0], s[1])) {
				return p;
			}
			if (extendSecond) return p2;
			else {
				assert(false && "You didn't check the segments for intersection before calling this, did you?");
				return new WorldPoint(0, 0);
			}
		}

		// t = (q - p) x s / (r x s)
		const t = cross(q.x - p.x, q.y - p.y, s[0], s[1]) / rxs;

		// u = (q - p) x r / (r x s)
		const u = cross(q.x - p.x, q.y - p.y, r[0], r[1]) / rxs;

		// 4. If r x s != 0 and 0 <= t <= 1 and 0 <= u <= 1
		// the two line segments meet at the point p + t r = q + u s.
		const tWithinEndpoints: boolean = 0 <= t && t <= 1;
		const uWithinEndpoints: boolean = 0 <= u && u <= 1;
		if (!isZero(rxs) && tWithinEndpoints && (extendSecond || uWithinEndpoints)) {
			// We can calculate the intersection point using either t or u.
			return new WorldPoint(p.x + t * r[0], p.y + t * r[1]);
		}
		// if we got here, the lines are parallel and there's a non-zero perpendicular distance between them
		assert(false && "You didn't check the segments for intersection before calling this, did you?");
		return new WorldPoint(0, 0);
	}

	/**
	 * Given three colinear points p, q, r, the function checks if
	 * point q lies on line segment 'pr'
	 */
	export function onSegment(p: WorldPoint, q: WorldPoint, r: WorldPoint): boolean {
		if (orientation(p, q, r) != 0) {
			return false;
		}
		if (
			q.x <= Math.max(p.x, r.x) &&
			q.x >= Math.min(p.x, r.x) &&
			q.y <= Math.max(p.y, r.y) &&
			q.y >= Math.min(p.y, r.y)
		) {
			return true;
		} else {
			return false;
		}
	}

	/** @returns the modulus (vectorial length) of a point: this is the distance from origin to the point */
	export function modulus<Point extends IPoint>(p: Point): number {
		const x = p.x;
		const y = p.y;
		return Math.sqrt(x * x + y * y);
	}

	/** @returns the direction from (0,0) to point p, in radians, CCW direction */
	export function pointDirection<Point extends IPoint>(p: Point): number {
		// normalize:
		const _1_len = 1.0 / modulus(p);
		const x = p.x * _1_len;
		const y = p.y * _1_len;
		if (x >= 0) return Math.asin(y);
		else {
			const s = y >= 0 ? 1 : -1;
			return -Math.asin(y) + Math.PI * s;
		}
	}

	// returns the distance between two points
	export function distance<Point extends IPoint>(p1: Point, p2: Point): number {
		const dx = p2.x - p1.x;
		const dy = p2.y - p1.y;
		return Math.sqrt(dx * dx + dy * dy);
	}
} //namespace
