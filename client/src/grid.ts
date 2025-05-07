import { GridPoint, ScreenPoint, Transform, WorldPoint } from "./math/math";
import { fmod } from "./math/functions";

export class Grid {
	private tr_: Transform;

	constructor(private readonly size_: number, private readonly winW_: number, private readonly winH_: number) {}

	setTransform(t: Transform): void {
		this.tr_ = t;
	}
	getTransform(): Transform {
		return this.tr_;
	}
	draw(): void {
		const squareSz = this.size_ * this.tr_.scale;
		const nSqX = Math.ceil(this.winW_ / squareSz);
		const nSqY = Math.ceil(this.winH_ / squareSz);
		let offsX = fmod(this.tr_.transX, this.size_);
		if (offsX < 0) offsX += this.size_;
		offsX *= this.tr_.scale;
		let offsY = fmod(this.tr_.transY, this.size_);
		if (offsY < 0) offsY += this.size_;
		offsY *= this.tr_.scale;

		// draw horizontal lines
		for (let i = 0; i < nSqY; i++) {
			const y = offsY + i * squareSz;
			// FIXME draw
			// Shape2D::get()->drawLine({0, y}, {winW_, y}, 0, Colors::GRID);
		}

		// draw vertical lines
		for (let i = 0; i < nSqX; i++) {
			const x = offsX + i * squareSz;
			// FIXME draw
			// Shape2D::get()->drawLine({x, 0}, {x, winH_}, 0, Colors::GRID);
		}
	}

	cellSize(): number {
		return this.size_;
	}
	gridToScreen(p: GridPoint): ScreenPoint {
		const sX = Math.floor((p.x * this.size_ + this.tr_.transX) * this.tr_.scale);
		const sY = Math.floor((p.y * this.size_ + this.tr_.transY) * this.tr_.scale);
		return new ScreenPoint(sX, sY);
	}

	gridToWorld(p: GridPoint): WorldPoint {
		return new WorldPoint(p.x * this.size_, p.y * this.size_);
	}

	screenToGrid(p: ScreenPoint): GridPoint {
		return this.worldToGrid(p.toWorld(this.tr_));
	}

	worldToGrid(wp: WorldPoint): GridPoint {
		let gX = wp.x / this.size_;
		let gY = wp.y / this.size_;
		const ret = new GridPoint(Math.floor(gX), Math.floor(gY), 0);
		gX = gX - Math.floor(gX);
		if (Math.abs(gX) > 0.5) {
			ret.x += gX > 0 ? 1 : -1;
			gX = 1.0 - Math.abs(gX);
		}
		gY = gY - Math.floor(gY);
		if (Math.abs(gY) > 0.5) {
			ret.y += gY > 0 ? 1 : -1;
			gY = 1.0 - Math.abs(gY);
		}
		ret.distance = Math.sqrt(gX * gX + gY * gY);
		return ret;
	}
}
