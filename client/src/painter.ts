import { Color } from "./color";
import { lineMath } from "./math/line-math";
import { ScreenPoint } from "./math/math";

export namespace Paint {
	export function loadResources(): void {
		// load textures and shit
	}

	// draws an arrow from point A to B with a head of size [headSize] pixels and [headAperture] aperture
	export function paintArrow(
		from: ScreenPoint,
		to: ScreenPoint,
		headSize: number,
		headAperture: number,
		color: Color,
	): void {
		const dir = new ScreenPoint(to.x - from.x, to.y - from.y);
		if (dir.x == 0 && dir.y == 0) {
			return;
		}
		const angle = lineMath.pointDirection(dir) + Math.PI;

		// draw arrow body:
		// FIXME draw line
		// Shape2D::get()->drawLine(from, to, 0.f, c);

		// draw arrow head:
		const pL = new ScreenPoint(
			to.x + Math.cos(angle + headAperture * 0.5) * headSize,
			to.y + Math.sin(angle + headAperture * 0.5) * headSize,
		);
		const pR = new ScreenPoint(
			to.x + Math.cos(angle - headAperture * 0.5) * headSize,
			to.y + Math.sin(angle - headAperture * 0.5) * headSize,
		);
		// FIXME draw line
		// Shape2D::get()->drawLine(to, pL, 0.f, c);
		// Shape2D::get()->drawLine(to, pR, 0.f, c);
	}
}
