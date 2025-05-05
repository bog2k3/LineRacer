import { Vector } from "./vector";

/** Represents an infinite axis with the implicit equation y = m*x + n */
export class Axis {
	constructor(public m: number, public n: number) {}

	static fromPointAndDirection(point: Vector, direction: Vector): Axis {
		const m = direction.y / direction.x;
		const n = point.y - m * point.x;
		return new Axis(m, n);
	}

	/**
	 * @returns -1 if the point is on the negative side of the axis (y < mx + n),
	 * 			+1 if the point is on the positive side of the axis (y > mx + n),
	 * 			 0 if the point is on the axis (y = mx + n).
	 */
	pointSide(point: Vector): number {
		return Math.sign(point.y - this.m * point.x - this.n);
	}
}
