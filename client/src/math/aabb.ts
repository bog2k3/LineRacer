import { Axis } from "./axis";
import { Vector } from "./vector";

export class AABB {
	vMin = new Vector(Infinity, Infinity);
	vMax = new Vector(-Infinity, -Infinity);

	constructor(vMin?: Vector, vMax?: Vector) {
		if (vMin) {
			this.vMin = vMin;
		}
		if (vMax) {
			this.vMax = vMax;
		}
	}

	copy(): AABB {
		return new AABB(this.vMin.copy(), this.vMax.copy());
	}

	/** returns an empty AABB */
	static empty(): AABB {
		return new AABB();
	}

	isEmpty(): boolean {
		return this.vMin.x > this.vMax.x || this.vMin.y > this.vMax.y;
	}

	center(): Vector {
		return this.vMin.add(this.vMax).scale(0.5);
	}

	size(): Vector {
		return new Vector(this.vMax.x - this.vMin.x, this.vMax.y - this.vMin.y);
	}

	/** Qualifies this AABB against an axis
	 * Returns:
	 * 	-1 if the entire AABB is on the negative side of the axis, where y<=mx+n (even if at least one vertex is exactly on the axis),
	 * 	+1 if the entire AABB is on the positive side of the axis, where y>=mx+n (even if at least one vertex is exactly on the axis),
	 * 	 0 if the AABB spans both sides of the axis (some vertices are strictly on the positive side, and some strictly on the negative).
	 */
	qualifyAxis(axis: Axis): number {
		const verts: Vector[] = [
			this.vMin,
			new Vector(this.vMin.x, this.vMax.y),
			this.vMax,
			new Vector(this.vMax.x, this.vMin.y),
		];
		let allPositive = true;
		let allNegative = true;
		for (let i = 0; i < 8; i++) {
			const q = axis.pointSide(verts[i]);
			if (q > 0) {
				allNegative = false;
			} else if (q < 0) {
				allPositive = false;
			}
		}
		return allPositive ? +1 : allNegative ? -1 : 0;
	}

	intersectsCircle(center: Vector, radius: number): boolean {
		if (
			center.x + radius <= this.vMin.x ||
			center.y + radius <= this.vMin.y ||
			center.x - radius >= this.vMax.x ||
			center.y - radius >= this.vMax.y
		) {
			return false;
		}
		if ((center.x > this.vMin.x && center.x < this.vMax.x) || (center.y > this.vMin.y && center.y < this.vMax.y)) {
			return true;
		}
		const rsq = radius * radius;
		return (
			center.sub(this.vMin).lengthSq() < rsq ||
			center.sub(this.vMax).lengthSq() < rsq ||
			center.sub(new Vector(this.vMin.x, this.vMax.y)).lengthSq() < rsq ||
			center.sub(new Vector(this.vMax.x, this.vMin.y)).lengthSq() < rsq
		);
	}

	/** expands this AABB to include the new point(s) */
	expandInPlace(...ps: Vector[]): this {
		const axes = ["x", "y"];
		for (let p of ps) {
			for (let i = 0; i < 3; i++) {
				if (p[axes[i]] < this.vMin[axes[i]]) {
					this.vMin[axes[i]] = p[axes[i]];
				}
				if (p[axes[i]] > this.vMax[axes[i]]) {
					this.vMax[axes[i]] = p[axes[i]];
				}
			}
		}
		return this;
	}

	/** creates a new AABB that is expanded to include the new point(s) */
	expand(...ps: Vector[]): AABB {
		const a = this.copy();
		a.expandInPlace(...ps);
		return a;
	}

	/** creates a new AABB that is the reunion of these two (includes all the points within both) */
	union(other: AABB): AABB {
		return this.expand(other.vMin, other.vMax);
	}

	unionInPlace(other: AABB): this {
		return this.expandInPlace(other.vMin, other.vMax);
	}

	/** creates a new AABB from the intersection of this and another AABB (will contain only points that are contained in both) */
	intersection(other: AABB): AABB {
		if (
			other.vMin.x >= this.vMax.x ||
			other.vMax.x <= this.vMin.x ||
			other.vMin.y >= this.vMax.y ||
			other.vMax.y <= this.vMin.y
		) {
			return AABB.empty();
		} else {
			return new AABB(
				new Vector(Math.max(this.vMin.x, other.vMin.x), Math.max(this.vMin.y, other.vMin.y)),
				new Vector(Math.min(this.vMax.x, other.vMax.x), Math.min(this.vMax.y, other.vMax.y)),
			);
		}
	}

	/** offsets this AABB by a given amount */
	offsetInPlace(offs: Vector): this {
		this.vMin.addInPlace(offs);
		this.vMax.addInPlace(offs);
		return this;
	}

	/** offsets this AABB by a given amount */
	offset(offs: Vector): AABB {
		const aabb = this.copy();
		aabb.offsetInPlace(offs);
		return aabb;
	}
}
