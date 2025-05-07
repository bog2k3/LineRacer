import { Grid } from "./grid";
import { lineMath } from "./math/line-math";
import { GridPoint, ScreenPoint, WorldPoint } from "./math/math";
import { Vector } from "./math/vector";
import { TrackPartition } from "./track-partition";
import { assert } from "./utils/assert";
import { WorldArea } from "./world-area";

export enum TrackDesignStep {
	DRAW,
	STARTLINE,
}

export type StartPosition = {
	position: GridPoint;
	direction: Vector;
};

export type TrackIntersectInfo = {
	point: WorldPoint;
	polyIndex: number;
};

type StartLine = {
	p1: WorldPoint;
	p2: WorldPoint;
	startPositions: StartPosition[];
	orientation: number;
	isValid: boolean;
};

const PARTITION_CELL_SPAN = 5;
const DEBUG_CODE_TO_TEST_POINT_INSIDE_POLYGON = false;
const DEBUG_CODE_TO_TEST_PARTITIONING = false;
const DEBUG_CODE_TO_TEST_PARTITION_DENSITY = false;

export class Track {
	private partition_: TrackPartition;
	private designMode_ = false;
	private pointerPressed_ = false;
	private currentPolyIdx_ = 0;
	private floatingVertex_: WorldPoint;
	private designStep_ = TrackDesignStep.DRAW;
	private startLine_: StartLine;
	/** track consists of two closed polygons, one inner and one outer */
	private polyVertex_: WorldPoint[][] = [[], []];
	private polyOrientation_: number[] = [0, 0];

	/** @param resolution : this is relative to grid's cell size, a resolution of 2 means the track vertices are twice as dense as the grid */
	constructor(
		private readonly grid_: Grid,
		private readonly worldArea_: WorldArea,
		private readonly resolution_: number,
	) {
		this.partition_ = new TrackPartition(this.worldArea_, this.grid_, PARTITION_CELL_SPAN);
	}

	grid(): Grid {
		return this.grid_;
	}

	worldArea(): WorldArea {
		return this.worldArea_;
	}

	draw(): void {
		const vPoints: ScreenPoint[] = new Array(Math.max(this.polyVertex_[0].length, this.polyVertex_[1].length));
		for (let i = 0; i < 2; i++) {
			if (this.polyVertex_[i].length < 2) continue;
			for (let j = 0; j < this.polyVertex_[i].length; j++) {
				const sp: ScreenPoint = this.polyVertex_[i][j].toScreen(this.grid_.getTransform());
				vPoints.push(sp);
			}
			// FIXME drawLineStrip()
			// Shape2D::get().drawLineStrip(vPoints.data(), vPoints.length, 0, Colors::TRACK);
		}
		if (
			this.designMode_ &&
			this.designStep_ == TrackDesignStep.DRAW &&
			this.polyVertex_[this.currentPolyIdx_].length > 0
		) {
			const p1: ScreenPoint = this.polyVertex_[this.currentPolyIdx_]
				.slice(-1)[0]
				.toScreen(this.grid_.getTransform());
			const p2: ScreenPoint = this.floatingVertex_.toScreen(this.grid_.getTransform());
			// FIXME drawLine()
			// Shape2D::get().drawLine(p1, p2, 0, Colors::TRACK_TRANSP);

			// if snapping close, draw the snap anchor
			if (
				this.polyVertex_[this.currentPolyIdx_].length > 2 &&
				this.floatingVertex_ == this.polyVertex_[this.currentPolyIdx_][0]
			) {
				const snapAnchorRadius = 3;
				const anchor: ScreenPoint = this.floatingVertex_
					.toScreen(this.grid_.getTransform())
					.subInPlace(new ScreenPoint(snapAnchorRadius, snapAnchorRadius));
				// FIXME drawRectangleFilled()
				// Shape2D::get().drawRectangleFilled(anchor, 0, {2*snapAnchorRadius+1, 2*snapAnchorRadius+1}, Colors::TRACK_SNAP);
			}
		}
		if (this.startLine_.isValid) {
			const s1: ScreenPoint = this.startLine_.p1.toScreen(this.grid_.getTransform());
			const s2: ScreenPoint = this.startLine_.p2.toScreen(this.grid_.getTransform());
			// FIXME drawLine()
			// Shape2D::get().drawLine(s1, s2, 0, Colors::STARTLINE);
			for (let spos of this.startLine_.startPositions) {
				const p1: ScreenPoint = this.grid_.gridToScreen(spos.position);
				const p2: ScreenPoint = this.grid_.gridToScreen(
					new GridPoint(spos.position.x + spos.direction.x, spos.position.y + spos.direction.y),
				);
				// FIXME drawArrow()
				// Painter::paintArrow(p1, p2, 10 * this.grid_.getTransform().scale, M_PI/6, Colors::STARTLINE);
			}
		}

		// #if 0 || DEBUG_CODE_TO_TEST_POINT_INSIDE_POLYGON
		// 	static std::vector<std::pair<WorldPoint, bool>> debugPoints;
		// 	static int sx = 0;
		// 	static int sy = 0;
		// 	if (this.designMode_ && currentPolyIdx_ == 1) {
		// 		WorldPoint tl = this.grid_.gridToWorld(worldArea_.topLeft());
		// 		WorldPoint br = this.grid_.gridToWorld(worldArea_.bottomRight());
		// 		while (sy+tl.y < br.y) {
		// 			while (sx+tl.x < br.x) {
		// 				WorldPoint wp{tl.x + sx, tl.y + sy};
		// 				bool green = pointInsidePolygon(wp, 0);
		// 				debugPoints.push({wp, green});
		// 				sx+=2;
		// 			}
		// 			sy+=2;
		// 			sx = 0;
		// 		}
		// 		/*if (debugPoints.length < 10000) {
		// 			WorldPoint tl = grid_.gridToWorld(worldArea_.topLeft());
		// 			WorldPoint br = grid_.gridToWorld(worldArea_.bottomRight());
		// 			WorldPoint wp{rand() / (float)RAND_MAX * (br.x-tl.x) + tl.x, rand() / (float)RAND_MAX * (br.y-tl.y) + tl.y};
		// 			bool green = pointInsidePolygon(wp, 0) && !pointInsidePolygon(wp, 1);
		// 			debugPoints.push({wp, green});
		// 		}*/
		// 		//SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
		// 		for (auto &p : debugPoints) {
		// 			if (p.second)
		// 				Colors::GREEN_TR.set(r);
		// 			else
		// 				Colors::RED_TR.set(r);
		// 			SDL_Rect rc{p.first.x, p.first.y, 2, 2};
		// 			SDL_RenderDrawRect(r, &rc);
		// 		}
		// 		//SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
		// 	} else {
		// 		debugPoints.clear();
		// 		sx = sy = 0;
		// 	}
		// #endif

		// #if 0 || DEBUG_CODE_TO_TEST_PARTITIONING
		// 	for (let i=0; i<partition_.cells.length; i++)
		// 		for (let j=0; j<partition_.cells[i].length; j++) {
		// 			GridPoint tl {j*partition_.relativeCellSize_, i*partition_.relativeCellSize_};
		// 			tl.x += worldArea_.topLeft().x;
		// 			tl.y += worldArea_.topLeft().y;
		// 			GridPoint br {(j+1)*partition_.relativeCellSize_, (i+1)*partition_.relativeCellSize_};
		// 			br.x += worldArea_.topLeft().x;
		// 			br.y += worldArea_.topLeft().y;
		// 			ScreenPoint stl = grid_.gridToScreen(tl);
		// 			ScreenPoint sbr = grid_.gridToScreen(br);
		// 			SDL_Rect rc { stl.x, stl.y, sbr.x-stl.x-1, sbr.y-stl.y-1 };
		// 			if (partition_.cells[i][j].length)
		// 				Colors::GREEN_TR.set(r);
		// 			else
		// 				Colors::RED_TR.set(r);
		// 			SDL_RenderFillRect(r, &rc);
		// 		}
		// #endif

		// #if 0 || DEBUG_CODE_TO_TEST_PARTITION_DENSITY
		// 	auto verts = partition_.getVerticesInArea(floatingVertex_, floatingVertex_);
		// 	Colors::RED.set(r);
		// 	for (auto &v : verts) {
		// 		if (v.second + 1 < this.polyVertex_[v.first].length) {
		// 			ScreenPoint p1 = this.polyVertex_[v.first][v.second].toScreen(grid_.getTransform());
		// 			ScreenPoint p2 = this.polyVertex_[v.first][v.second+1].toScreen(grid_.getTransform());
		// 			SDL_RenderDrawLine(r, p1.x, p1.y, p2.x, p2.y);
		// 		}
		// 	}
		// #endif
	}

	/**  clears the track */
	reset(): void {
		this.currentPolyIdx_ = 0;
		this.polyVertex_[0] = [];
		this.polyVertex_[1] = [];
		this.partition_.clear();
		this.startLine_.isValid = false;
	}

	enableDesignMode(enable: boolean): void {
		this.currentPolyIdx_ = 0;
		this.designMode_ = enable;
		this.designStep_ = TrackDesignStep.DRAW;
	}

	setDesignStep(step: TrackDesignStep): void {}

	isInDesignMode(): boolean {
		return this.designMode_;
	}

	/** x and y are world-space coordinates */
	pointerMoved(x: number, y: number): void {
		if (!this.designMode_) return;
		this.floatingVertex_.x = x;
		this.floatingVertex_.y = y;
		switch (this.designStep_) {
			case TrackDesignStep.DRAW:
				if (this.pointerPressed_ && this.polyVertex_[this.currentPolyIdx_].length && this.validateVertex()) {
					// live drawing
					const minDist = this.grid_.cellSize() / this.resolution_;
					if (this.floatingVertex_.distanceTo(this.polyVertex_[this.currentPolyIdx_].slice(-1)[0]) >= minDist)
						this.pushVertex();
				} else {
					this.checkCloseSnap();
				}
				break;
			case TrackDesignStep.STARTLINE:
				this.updateStartLine();
				break;
		}
	}

	/** x and y are world-space coordinates */
	pointerTouch(on: boolean, x: number, y: number): void {
		if (!this.designMode_) return;
		this.pointerPressed_ = on;
		this.floatingVertex_.x = x;
		this.floatingVertex_.y = y;
		switch (this.designStep_) {
			case TrackDesignStep.DRAW:
				if (on && this.validateVertex()) {
					// TODO: bug here - detecting false collision with first line segment (which should indeed touch at the end)
					const closed: boolean = this.checkCloseSnap();
					// add the next vertex
					this.pushVertex();

					if (closed) {
						this.polyOrientation_[this.currentPolyIdx_] =
							lineMath.clockwiseness(
								this.polyVertex_[this.currentPolyIdx_],
								this.polyVertex_[this.currentPolyIdx_].length - 1,
							) > 0
								? +1
								: -1;
						if (this.currentPolyIdx_ == 0) this.currentPolyIdx_++;
						else this.designStep_ = TrackDesignStep.STARTLINE; // go to next step
					}
				}
				break;
			case TrackDesignStep.STARTLINE:
				if (on) {
					this.updateStartLine();
				} else if (this.startLine_.isValid) {
					this.enableDesignMode(false);
				}
				break;
		}
	}

	/** @returns true if the track is ready for the game */
	isReady(): boolean {
		return !this.designMode_ && this.startLine_.isValid;
	}

	getStartPositions(): StartPosition[] {
		return this.isReady() ? this.startLine_.startPositions : [];
	}

	/**
	 * @returns true if a line from grid point p1 to p2 intersects a track segment
	 * @param out_info: optional . intersection point and polygon index that was intersected will be stored in it
	 */
	intersectLine(p1: GridPoint, p2: GridPoint, out_info?: TrackIntersectInfo): boolean {
		return this.intersectLineW(this.grid_.gridToWorld(p1), this.grid_.gridToWorld(p2), out_info, false);
	}

	/** @returns true if a line from point p1 to p2 intersects a track segment */
	private intersectLineW(
		p1: WorldPoint,
		p2: WorldPoint,
		out_info?: TrackIntersectInfo,
		skipLastSegment = false,
	): boolean {
		const topLeft = new WorldPoint(Math.min(p1.x, p2.x), Math.min(p1.y, p2.y));
		const bottomRight = new WorldPoint(Math.max(p1.x, p2.x), Math.max(p1.y, p2.y));
		const vertices: Set<[number, number]> = this.partition_.getVerticesInArea(topLeft, bottomRight);
		for (const v of vertices) {
			const polyIdx: number = v[0];
			const vIdx: number = v[1];
			if (
				skipLastSegment &&
				polyIdx == this.currentPolyIdx_ &&
				vIdx + 2 == this.polyVertex_[this.currentPolyIdx_].length
			) {
				continue; // we're not checking against last segment because that one is connected
			}
			if (vIdx + 1 < this.polyVertex_[polyIdx].length) {
				// check line following vertex
				if (
					lineMath.segmentIntersect(
						p1,
						p2,
						this.polyVertex_[polyIdx][vIdx],
						this.polyVertex_[polyIdx][vIdx + 1],
					)
				) {
					if (out_info) {
						out_info.point = lineMath.intersectionPoint(
							p1,
							p2,
							this.polyVertex_[polyIdx][vIdx],
							this.polyVertex_[polyIdx][vIdx + 1],
						);
						out_info.polyIndex = polyIdx;
					}
					return true;
				}
			}
		}
		return false;
	}

	/** @returns the number of points in which the arrow intersects the track polygons */
	intersectionsCount(p1: GridPoint, p2: GridPoint): number {
		const wp1: WorldPoint = this.grid_.gridToWorld(p1);
		const wp2: WorldPoint = this.grid_.gridToWorld(p2);
		const topLeft = new WorldPoint(Math.min(wp1.x, wp2.x), Math.min(wp1.y, wp2.y));
		const bottomRight = new WorldPoint(Math.max(wp1.x, wp2.x), Math.max(wp1.y, wp2.y));
		const vertices: Set<[number, number]> = this.partition_.getVerticesInArea(topLeft, bottomRight);
		let count = 0;
		for (let v of vertices) {
			const polyIdx: number = v[0];
			const vIdx: number = v[1];
			if (vIdx + 1 == this.polyVertex_[polyIdx].length) continue; // this was the last vertex, we skip it
			// we check forward, from this vertex to the next
			const v1: WorldPoint = this.polyVertex_[polyIdx][vIdx];
			const v2: WorldPoint = this.polyVertex_[polyIdx][vIdx + 1];
			const res: lineMath.IntersectionResult = lineMath.segmentIntersect(v1, v2, wp1, wp2);
			if (
				res == lineMath.IntersectionResult.INTERSECT_NONE ||
				res == lineMath.IntersectionResult.INTERSECT_OVERLAP
			)
				continue;
			let consider = false;
			if (res == lineMath.IntersectionResult.INTERSECT_MIDDLE) consider = true;
			else if (res == lineMath.IntersectionResult.INTERSECT_ENDPOINT1)
				// v1 is on wp1.wp2 we count it only if the line is going downward
				consider = v2.y > v1.y;
			else if (res == lineMath.IntersectionResult.INTERSECT_ENDPOINT2)
				// v2 is on wp1.wp2, we count it only if the line is going upward
				consider = v2.y < v1.y;

			if (consider) count++;
		}
		return count;
	}

	/** @returns true if the point is inside the closed polygon */
	pointInsidePolygon(p: WorldPoint, polyIndex: number): boolean {
		assert(polyIndex >= 0 && polyIndex <= 1);
		if (this.polyVertex_[polyIndex].length < 3) return false;
		if (!this.worldArea_.containsPoint(p)) return false;
		const polyOrientation: number = this.polyOrientation_[polyIndex];
		// draw an imaginary horizontal line from the left limit of worldArea through point p
		// then see how many edges from the test polygon it intersects
		// odd means point is inside, even means it's outside
		const start = new WorldPoint(this.grid_.gridToWorld(this.worldArea_.topLeft()).x - 1, p.y);
		const verts: Set<[number, number]> = this.partition_.getVerticesInArea(
			new WorldPoint(start.x, p.y - 1),
			new WorldPoint(p.x + 1, p.y + 1),
		);
		let wn = 0;
		for (const v of verts) {
			if (v[0] != polyIndex) continue;
			// test edge before vertex
			if (v[1] > 0) {
				const v1: WorldPoint = this.polyVertex_[polyIndex][v[1] - 1];
				const v2: WorldPoint = this.polyVertex_[polyIndex][v[1]];
				if (v1.x >= p.x && v2.x >= p.x) continue;
				const res: lineMath.IntersectionResult = lineMath.segmentIntersect(v1, v2, start, p);
				if (
					res == lineMath.IntersectionResult.INTERSECT_NONE ||
					res == lineMath.IntersectionResult.INTERSECT_OVERLAP
				)
					continue;
				let count = false;
				if (res == lineMath.IntersectionResult.INTERSECT_MIDDLE) count = true;
				else if (res == lineMath.IntersectionResult.INTERSECT_ENDPOINT1)
					// v1 is on the horizontal line, we count it only if the line is going downward
					count = v2.y > v1.y;
				else if (res == lineMath.IntersectionResult.INTERSECT_ENDPOINT2)
					// v2 is on the horizontal line, we count it only if the line is going upward
					count = v2.y < v1.y;

				if (!count) continue;
				const side = lineMath.orientation(v1, v2, p);
				wn += side == polyOrientation ? +1 : -1;
			}
		}
		return wn != 0;
	}

	/**
	 * crossing index indicates the point where the line [p1, p2] intersects a track contour;
	 * the first number represents the segment index, and the second number represents the position on that segment
	 * first is contour index, second is the crossing index into that contour
	 */
	computeCrossingIndex(p1: GridPoint, p2: GridPoint): number[] {
		const wp1: WorldPoint = this.grid_.gridToWorld(p1);
		const wp2: WorldPoint = this.grid_.gridToWorld(p2);
		const topLeft = new WorldPoint(Math.min(wp1.x, wp2.x), Math.min(wp1.y, wp2.y));
		const bottomRight = new WorldPoint(Math.max(wp1.x, wp2.x), Math.max(wp1.y, wp2.y));
		const verts: Set<[number, number]> = this.partition_.getVerticesInArea(topLeft, bottomRight);
		for (const v of verts) {
			const pIndex = v[0];
			const vIndex = v[1];
			if (vIndex + 1 == this.polyVertex_[pIndex].length) continue;
			const sp1: WorldPoint = this.polyVertex_[pIndex][vIndex];
			const sp2: WorldPoint = this.polyVertex_[pIndex][vIndex + 1];
			if (lineMath.segmentIntersect(sp1, sp2, wp1, wp2)) {
				const crossPoint: WorldPoint = lineMath.intersectionPoint(sp1, sp2, wp1, wp2);
				const segLength: number = lineMath.distance(sp1, sp2);
				const crossLength: number = lineMath.distance(sp1, crossPoint);
				assert(crossLength <= segLength);
				return [pIndex, vIndex + crossLength / segLength];
			}
		}
		return [-1, -1];
	}

	/**
	 * @returns the "forward" vertex direction for a given polygon with respect to the startLine's direction;
	 * 	if the startline points in the same direction as the polygon winding, will return +1, otherwise -1;
	 * @returns 0 if the request is not valid in the current state
	 */
	polyDirection(polyIndex: number): number {
		assert(polyIndex <= 1);
		if (!this.startLine_.isValid) return 0;
		return this.polyOrientation_[polyIndex] == this.startLine_.orientation ? +1 : -1;
	}

	/** @returns the length (in vertices) of the specified polygon (0: outer poly, 1: inner poly) */
	polyLength(index: number): number {
		assert(index <= 1);
		return this.polyVertex_[index].length;
	}

	/** @returns the [index] vertex from [poly] polygon */
	polyVertex(poly: number, index: number): WorldPoint {
		assert(poly <= 1 && index < this.polyVertex_[poly].length);
		return this.polyVertex_[poly][index];
	}

	/**
	 * checks if a line [from, to] crosses the start-line
	 * 		* checks segment if extended==false
	 * 		* checks infinite line if extended==true
	 * @returns +1 if the line crosses the start line from back to front
	 * @returns -1 if the line crosses the start line from front to back
	 * @returns 0 if they don't cross
	 * @param out_Point: if not null, will be filled with the intersection point
	 */
	checkStartLineCross(from: GridPoint, to: GridPoint, extended: boolean, out_point?: WorldPoint): number {
		const fw: WorldPoint = this.grid_.gridToWorld(from);
		const tw: WorldPoint = this.grid_.gridToWorld(to);
		let res: lineMath.IntersectionResult;
		if (extended) {
			res = lineMath.segmentIntersectLine(fw, tw, this.startLine_.p1, this.startLine_.p2);
		} else {
			res = lineMath.segmentIntersect(fw, tw, this.startLine_.p1, this.startLine_.p2);
		}
		if (
			res == lineMath.IntersectionResult.INTERSECT_NONE ||
			res == lineMath.IntersectionResult.INTERSECT_OVERLAP ||
			res == lineMath.IntersectionResult.INTERSECT_ENDPOINT1
		) {
			return 0;
		}
		if (out_point) {
			out_point.assign(lineMath.intersectionPoint(fw, tw, this.startLine_.p1, this.startLine_.p2, extended));
		}
		const dot =
			(to.x - from.x) * this.startLine_.startPositions[0].direction.x +
			(to.y - from.y) * this.startLine_.startPositions[0].direction.y;
		return dot > 0 ? +1 : -1;
	}

	// -------------------------- PRIVATE AREA ------------------------------- //

	private checkCloseSnap(): boolean {
		// if the floating vertex comes to within a grid square of the starting point, snap it to close the loop
		if (
			this.polyVertex_[this.currentPolyIdx_].length > 2 &&
			this.floatingVertex_.distanceTo(this.polyVertex_[this.currentPolyIdx_][0]) < this.grid_.cellSize()
		) {
			this.floatingVertex_ = this.polyVertex_[this.currentPolyIdx_][0];
			return true;
		}
		return false;
	}

	private validateVertex(): boolean {
		// checks whether the floating vertex is within world's boundary
		// also when drawing second (inner) contour, it checks if the vertex is within the outer contour
		// also checks that the new line segment won't intersect any existing line segments

		// check world boundary:
		if (!this.worldArea_.containsPoint(this.floatingVertex_)) return false;

		// check intersection:
		if (this.polyVertex_[this.currentPolyIdx_].length) {
			const prevVertex: WorldPoint = this.polyVertex_[this.currentPolyIdx_].slice(-1)[0];
			if (this.intersectLineW(prevVertex, this.floatingVertex_, undefined, true)) return false;
		}

		// check second contour to be inside first:
		if (this.currentPolyIdx_ == 1 && this.polyVertex_[1].length == 0) {
			if (!this.pointInsidePolygon(this.floatingVertex_, 0)) return false;
		}

		return true;
	}

	private pushVertex(): void {
		const crtIndex: number = this.polyVertex_[this.currentPolyIdx_].length;
		this.partition_.addVertex([this.currentPolyIdx_, crtIndex], this.floatingVertex_);
		if (crtIndex > 0) {
			this.partition_.addSegment(
				[this.currentPolyIdx_, [crtIndex - 1, crtIndex]],
				this.polyVertex_[this.currentPolyIdx_].slice(-1)[0],
				this.floatingVertex_,
			);
		}
		this.polyVertex_[this.currentPolyIdx_].push(this.floatingVertex_);
	}

	private updateStartLine(): void {
		this.startLine_.isValid = false;
		const touch: GridPoint = this.grid_.worldToGrid(this.floatingVertex_);
		if (
			!this.pointInsidePolygon(this.grid_.gridToWorld(touch), 0) ||
			this.pointInsidePolygon(this.grid_.gridToWorld(touch), 1)
		) {
			return;
		}

		// sweep all 4 possible directions and chose the one that intersects both polygons at the shortest distance
		const directions: number[][] = [
			[1, 0],
			[0, 1],
			[1, 1],
			[-1, 1],
		];
		const intersections: {
			steps: number;
			distance: number;
			polyId: number;
			p1: WorldPoint;
			p2: WorldPoint;
		}[] = new Array(4);
		for (let i = 0; i < 4; i++) {
			intersections[i] = {
				steps: 0,
				distance: 0,
				polyId: -1,
				p1: new WorldPoint(0, 0),
				p2: new WorldPoint(0, 0),
			};
			for (let j = -1; j <= 1; j += 2) {
				// when j==1 we sweep positive, j==-1 we sweep negative
				const p: GridPoint = touch;
				const tIntersect: TrackIntersectInfo = {
					point: null,
					polyIndex: -1,
				};
				do {
					p.x += directions[i][0] * j;
					p.y += directions[i][1] * j;
					intersections[i].steps++;
					tIntersect.point = j == -1 ? intersections[i].p1 : intersections[i].p2;
				} while (!this.intersectLine(touch, p, tIntersect));
				if (j == -1) intersections[i].polyId = tIntersect.polyIndex;
				else if (intersections[i].polyId == tIntersect.polyIndex) {
					intersections[i].steps = 10000; // we don't count this one if it doesn't intersect both polygons
				}
			}
			intersections[i].distance = intersections[i].steps;
			if (i >= 2) intersections[i].distance *= 1.4142; // because last two are diagonals
		}
		let imin = 0;
		for (let i = 1; i < 4; i++) {
			if (intersections[i].distance < intersections[imin].distance) imin = i;
		}
		if (intersections[imin].steps == 10000) return; // no valid start line found
		this.startLine_.isValid = true;
		this.startLine_.p1 = intersections[imin].p1;
		this.startLine_.p2 = intersections[imin].p2;
		// compute start line orientation:
		const outer: WorldPoint = intersections[imin].polyId == 0 ? this.startLine_.p1 : this.startLine_.p2; // intersection point on outer polygon
		const inner: WorldPoint = intersections[imin].polyId == 0 ? this.startLine_.p2 : this.startLine_.p1; // intersection point on inner polygon
		this.startLine_.orientation = lineMath.orientation(inner, outer, this.floatingVertex_);
		// compute valid starting arrows:
		this.startLine_.startPositions = [];
		const p: GridPoint = this.grid_.worldToGrid(this.startLine_.p1);
		for (let i = 0; i < intersections[imin].steps; p.x += directions[imin][0], p.y += directions[imin][1], i++) {
			if (
				!this.pointInsidePolygon(this.grid_.gridToWorld(p), 0) ||
				this.pointInsidePolygon(this.grid_.gridToWorld(p), 1)
			)
				continue;
			const CW: boolean =
				lineMath.orientation(intersections[imin].p1, intersections[imin].p2, this.floatingVertex_) == 1;
			const dirx: number = directions[imin][1] * (CW ? 1 : -1);
			const diry: number = directions[imin][0] * (CW ? -1 : 1);
			if (!this.intersectLine(p, new GridPoint(p.x + dirx, p.y + diry))) {
				// this is a valid start position and direction
				this.startLine_.startPositions.push({ position: p, direction: new Vector(dirx, diry) });
			}
		}
	}
}
