import { Color, Colors } from "./color";
import { Game, GameState } from "./game";
import { Grid } from "./grid";
import { lineMath } from "./math/line-math";
import { Arrow, GridPoint, ScreenPoint, WorldPoint } from "./math/math";
import { Vector } from "./math/vector";
import { PlayerType } from "./player";

class PossiblePoint {
	p: GridPoint;
	isWithinTrack = false;
	isValid = false;

	constructor(x: number, y: number) {
		this.p = new GridPoint(x, y);
	}
}

/**
 * Controller for local human player.
 * Handles local input events and translates them into Player actions
 */
export class HumanController {
	private pointerDown_ = false;
	private selectedPoint_: GridPoint;
	private hoverPoint_: GridPoint;
	private hasSelectedPoint_ = false;

	private possibleMoves_: PossiblePoint[] = [];

	constructor(private readonly game_: Game, private readonly grid_: Grid) {}

	onPointerMoved(where: GridPoint): void {
		if (
			!this.game_.activePlayer() ||
			this.game_.activePlayer().type != PlayerType.TYPE_HUMAN ||
			!this.game_.activePlayer().isTurnActive()
		) {
			return;
		}

		this.hoverPoint_ = where;
		if (this.pointerDown_ && this.isPointSelectable(where)) {
			this.selectedPoint_ = this.hoverPoint_;
			this.hasSelectedPoint_ = true;
		}
	}

	onPointerTouch(pressed: boolean): void {
		if (pressed == this.pointerDown_) return;
		this.pointerDown_ = pressed;

		if (
			!this.game_.activePlayer() ||
			this.game_.activePlayer().type != PlayerType.TYPE_HUMAN ||
			!this.game_.activePlayer().isTurnActive()
		) {
			return;
		}

		if (!this.isPointSelectable(this.hoverPoint_)) {
			return;
		}

		if (pressed) {
			this.selectedPoint_ = this.hoverPoint_;
		} else {
			// touch released
			if (
				this.hasSelectedPoint_ &&
				this.isValidMove(this.selectedPoint_) &&
				this.game_.activePlayer().actionPoint() == this.selectedPoint_
			) {
				// this is a confirmation
				this.game_.activePlayer().confirmNextPoint();
			} else {
				this.game_.activePlayer().selectNextPoint(this.selectedPoint_);
				this.hasSelectedPoint_ = true;
			}
		}
	}

	/** this is called by the game when a new turn starts */
	nextTurn(): void {
		// reset stuff
		this.pointerDown_ = false;
		this.hasSelectedPoint_ = false;
		// compute player's possible moves:
		const validMoves: Arrow[] = this.game_.activePlayer().validMoves();
		this.possibleMoves_.splice(0, this.possibleMoves_.length); // clear the list
		if (this.game_.state() == GameState.START_SELECTION) {
			for (const a of validMoves) this.possibleMoves_.push({ p: a.to, isValid: true, isWithinTrack: true });
			return;
		}
		const arrow = this.game_.activePlayer().lastArrow;
		const topLeft = new GridPoint(arrow.to.x + arrow.direction().x - 1, arrow.to.y + arrow.direction().y - 1);
		for (let i = 0; i < 9; i++) {
			const x = topLeft.x + i / 3;
			const y = topLeft.y + (i % 3);
			const pt = new PossiblePoint(x, y);
			pt.isValid = validMoves.some((a) => a.to == pt.p);
			pt.isWithinTrack = this.game_.isGPointOnTrack(pt.p);
			this.possibleMoves_.push(pt);
		}
	}

	draw(): void {
		if (
			!this.game_.activePlayer() ||
			this.game_.activePlayer().type != PlayerType.TYPE_HUMAN ||
			!this.game_.activePlayer().isTurnActive()
		) {
			return;
		}

		// render the possible moves
		for (const m of this.possibleMoves_) {
			let color = Colors.INVALID_MOVE;
			if (m.isValid) {
				if (m.isWithinTrack) color = Colors.VALID_MOVE;
				else color = Colors.OUT_TRACK_MOVE;
			}
			const to: ScreenPoint = this.grid_.gridToScreen(m.p);
			const POINT_RADIUS: number = Math.max(1.0, 3 * this.grid_.getTransform().scale);
			if (m.isValid) {
				const pos = new Vector(to.x - POINT_RADIUS, to.y - POINT_RADIUS);
				const size = new Vector(2 * POINT_RADIUS + 1, 2 * POINT_RADIUS + 1);
				if (this.hasSelectedPoint_ && this.selectedPoint_ == m.p) {
					// FIXME draw
					// Shape2D.get().drawRectangleFilled(pos, 0, size, *color);
				} else {
					// FIXME draw
					// Shape2D.get().drawRectangle(pos, 0, size, *color);
				}
			} else {
				// mark the invalid point with an X
				// FIXME draw
				// Shape2D.get().drawLine({to.x - POINT_RADIUS, to.y - POINT_RADIUS}, {to.x + POINT_RADIUS, to.y + POINT_RADIUS}, 0, *color);
				// Shape2D.get().drawLine({to.x - POINT_RADIUS, to.y + POINT_RADIUS}, {to.x + POINT_RADIUS, to.y - POINT_RADIUS}, 0, *color);
			}
		}
		if (this.game_.state() == GameState.PLAYING) {
			// if no point selected yet, draw the previous vector, else draw the new vector to the selected point
			const lastArrow = this.game_.activePlayer().lastArrow;
			const from: ScreenPoint = this.grid_.gridToScreen(lastArrow.to);
			const to: ScreenPoint = this.hasSelectedPoint_
				? this.grid_.gridToScreen(this.selectedPoint_)
				: this.grid_.gridToScreen(
						new GridPoint(
							lastArrow.to.x + lastArrow.direction().x,
							lastArrow.to.y + lastArrow.direction().y,
						),
				  );

			// FIXME draw
			// Painter.paintArrow(from, to, 10 * this.grid_.getTransform().scale, M_PI/6, Colors.UNCONFIRMED_ARROW);
		}
		// if player is off-track, highlight the contour area where he can re-enter
		// This must take into account the polygon winding direction (CW or CCW) and the start-line direction as well
		if (this.game_.activePlayer().offTrackData.offTrack) {
			const exitPolygon: number = this.game_.activePlayer().offTrackData.contourIndex;
			const exitSegIndex: number = Math.floor(this.game_.activePlayer().offTrackData.position);
			const exitSegWeight: number = this.game_.activePlayer().offTrackData.position - exitSegIndex;
			const requiredRedDistance: number = this.grid_.cellSize() * 10;
			const requiredGreenDistance: number = this.grid_.cellSize() * 7;
			const reentryDirection: number = -this.game_.track().polyDirection(exitPolygon); // reentry direction depends on polygon direction vs startLine direction

			for (let sign = +1; sign >= -1; sign -= 2) {
				const greenPart: boolean = sign == reentryDirection;
				let distance = 0;
				const requiredDist: number = greenPart ? requiredGreenDistance : requiredRedDistance;
				const c: Color = greenPart ? Colors.GREEN : Colors.RED;
				for (let i = 0; distance < requiredDist && i < this.game_.track().polyLength(exitPolygon) / 2; i++) {
					c.a = 255 * (1.0 - distance / requiredDist);
					let drawLength = 1.0;
					if (i == 0) {
						// this is the exit segment, we draw only a fraction of it
						drawLength = sign > 0 ? 1.0 - exitSegWeight : exitSegWeight;
						// when drawLength > 0, we draw from 0.0 to drawLength
						// when drawLength < 0, we draw from 1.0+drawLength to 1.0
					}
					const offs: number = sign < 0 ? 1 : 0;
					const i1: number =
						(exitSegIndex + this.game_.track().polyLength(exitPolygon) + offs + i * sign) %
						this.game_.track().polyLength(exitPolygon);
					const i2: number =
						(exitSegIndex + this.game_.track().polyLength(exitPolygon) + offs + (i + 1) * sign) %
						this.game_.track().polyLength(exitPolygon);
					const wp1: WorldPoint = this.game_.track().polyVertex(exitPolygon, i1);
					const wp2: WorldPoint = this.game_.track().polyVertex(exitPolygon, i2);
					// adjust if drawing part of the segment:
					if (drawLength != 1.0) {
						wp1.x = wp2.x + (wp1.x - wp2.x) * drawLength;
						wp1.y = wp2.y + (wp1.y - wp2.y) * drawLength;
					}
					distance += lineMath.distance(wp1, wp2);
					const sp1: ScreenPoint = wp1.toScreen(this.game_.track().grid().getTransform());
					const sp2: ScreenPoint = wp2.toScreen(this.game_.track().grid().getTransform());
					// FIXME draw
					// Shape2D.get().drawLine(sp1, sp2, 0, c);
				}
			}
		}
	}

	// returns true if the selected point is a valid move for the current player
	private isValidMove(p: GridPoint): boolean {
		if (
			!this.game_.activePlayer() ||
			this.game_.activePlayer().type != PlayerType.TYPE_HUMAN ||
			!this.game_.activePlayer().isTurnActive()
		)
			return false;

		for (const a of this.game_.activePlayer().validMoves()) if (a.to == p) return true;
		return false;
	}

	// returns true if the selected point is one of the 9 possible next positions for the current player, but not necesarily a valid move
	private isPointSelectable(p: GridPoint): boolean {
		for (let i = 0; i < 9; i++) if (p == this.possibleMoves_[i].p) return true;
		return false;
	}
}
