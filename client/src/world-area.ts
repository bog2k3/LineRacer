import { Grid } from "./grid";
import { GridPoint, ScreenPoint, WorldPoint } from "./math/math";
import { Vector } from "./math/vector";

export class WorldArea {
	constructor(
		private readonly grid_: Grid,
		private readonly topLeft_: GridPoint,
		private readonly bottomRight_: GridPoint,
	) {}

	draw(): void {
		const sTopLeft: ScreenPoint = this.grid_.gridToScreen(this.topLeft_);
		const sBottomRight: ScreenPoint = this.grid_.gridToScreen(this.bottomRight_);

		const thickness = 1;

		// left edge
		let pos = new Vector(sTopLeft.x - thickness, sTopLeft.y - thickness);
		let sz = new Vector(1 + 2 * thickness, sBottomRight.y - sTopLeft.y + 2 * thickness);
		// FIXME draw
		//Shape2D::get().drawRectangleFilled(pos, 0, sz, Colors::GRID);

		// top edge
		pos = new Vector(sTopLeft.x - thickness, sTopLeft.y - thickness);
		sz = new Vector(sBottomRight.x - sTopLeft.x + 2 * thickness, 1 + 2 * thickness);
		// FIXME draw
		//Shape2D::get().drawRectangleFilled(pos, 0, sz, Colors::GRID);

		// right edge
		pos = new Vector(sBottomRight.x - thickness, sTopLeft.y - thickness);
		sz = new Vector(1 + 2 * thickness, sBottomRight.y - sTopLeft.y + 2 * thickness);
		// FIXME draw
		//Shape2D::get().drawRectangleFilled(pos, 0, sz, Colors::GRID);

		// bottom edge
		pos = new Vector(sTopLeft.x - thickness, sBottomRight.y - thickness);
		sz = new Vector(sBottomRight.x - sTopLeft.x + 2 * thickness, 1 + 2 * thickness);
		// FIXME draw
		// Shape2D::get().drawRectangleFilled(pos, 0, sz, Colors::GRID);
	}

	containsPoint(wp: WorldPoint): boolean {
		const wTopLeft: WorldPoint = this.grid_.gridToWorld(this.topLeft_);
		const wBottomRight: WorldPoint = this.grid_.gridToWorld(this.bottomRight_);
		return wp.x > wTopLeft.x && wp.y > wTopLeft.y && wp.x < wBottomRight.x && wp.y < wBottomRight.y;
	}

	topLeft(): GridPoint {
		return this.topLeft_;
	}
	bottomRight(): GridPoint {
		return this.bottomRight_;
	}
}
