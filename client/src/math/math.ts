import { Vector } from "./vector";

export class Transform {
	/** screen to world unit ratio */
	scale = 1;
	/** world-space offset */
	transX = 0;
	/** workd-space offset */
	transY = 0;
}

export interface IPoint {
	x: number;
	y: number;
}

export class WorldPoint extends Vector implements IPoint {
	constructor(x: number, y: number) {
		super(x, y);
	}

	toScreen(tr: Transform): ScreenPoint {
		const sx = (this.x + tr.transX) * tr.scale;
		const sy = (this.y + tr.transY) * tr.scale;
		return new ScreenPoint(sx, sy);
	}

	distanceTo(p: WorldPoint): number {
		const xdif = this.x - p.x;
		const ydif = this.y - p.y;
		return Math.sqrt(xdif * xdif + ydif * ydif);
	}

	assign(p: WorldPoint): void {
		this.x = p.x;
		this.y = p.y;
	}
}

export class ScreenPoint extends Vector implements IPoint {
	constructor(x: number, y: number) {
		super(x, y);
	}

	toWorld(tr: Transform): WorldPoint {
		const wx = this.x / tr.scale - tr.transX;
		const wy = this.y / tr.scale - tr.transY;
		return new WorldPoint(wx, wy);
	}
}

export class GridPoint extends Vector implements IPoint {
	/** distance from actual pixel to the grid location, relative to grid size [0.0 .. sqrt(2)/2] */
	distance = 0;

	constructor(x: number, y: number, dist: number = 0) {
		super(x, y);
		this.distance = dist;
	}

	eq(p: GridPoint): boolean {
		return this.x == p.x && this.y == p.y;
	}
}

export class Arrow {
	constructor(public from: GridPoint, public to: GridPoint) {}

	length(): number {
		const dir = this.direction();
		const dx = Math.abs(dir.x);
		const dy = Math.abs(dir.y);
		return Math.max(dx, dy);
	}

	direction(): Vector {
		const x = this.to.x - this.from.x;
		const y = this.to.y - this.from.y;
		return new Vector(x, y);
	}

	static fromPointAndDir(p: GridPoint, dirX: number, dirY: number): Arrow {
		return new Arrow(p, new GridPoint(p.x + dirX, p.y + dirY));
	}
}
