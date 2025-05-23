import { Grid } from "./grid";
import { lineMath } from "./math/line-math";
import { WorldPoint } from "./math/math";
import { assert } from "./utils/assert";
import { WorldArea } from "./world-area";

type element_type = [number, number];
type cell_type = Set<element_type>;
type row_type = cell_type[];
export type segment_type = [number, [number, number]];

export class TrackPartition {
	private cells: row_type[];

	/** @param relativeCellSize_: how many grid cells is one partition cell wide and high? */
	constructor(
		private readonly worldArea: WorldArea,
		private readonly grid: Grid,
		private readonly relativeCellSize_: number,
	) {
		const rows: number = (worldArea.bottomRight().y - worldArea.topLeft().y) / this.relativeCellSize_ + 1;
		const cols: number = (worldArea.bottomRight().x - worldArea.topLeft().x) / this.relativeCellSize_ + 1;
		this.cells = new Array(rows);
		for (let i = 0; i < rows; i++) {
			this.cells[i] = new Array(cols);
			for (let j = 0; j < cols; j++) {
				this.cells[i][j] = new Set<element_type>();
			}
		}
	}

	// adds a vertex into space partitioning structure
	addVertex(index: element_type, v: WorldPoint): void {
		const coord = this.worldToCell(v);
		const row = coord[0];
		const col = coord[1];
		this.cells[row][col].add(index);
	}

	// adds a line segment into the partitioning structure -> its vertices will be added to all cells that the segment overlaps
	addSegment(seg: segment_type, p1: WorldPoint, p2: WorldPoint): void {
		const coord1 = this.worldToCell(p1);
		const coord2 = this.worldToCell(p2);
		const row1 = Math.min(coord1[0], coord2[0]);
		const col1 = Math.min(coord1[1], coord2[1]);
		const row2 = Math.max(coord1[0], coord2[0]);
		const col2 = Math.max(coord1[1], coord2[1]);
		// we need to check every cell in the interval (r1,c1)->(r2,c2) for intersection with the line segment
		for (let i = row1; i <= row2; i++)
			for (let j = col1; j <= col2; j++) {
				const c1 = this.cellToWorld(i, j); // top-left corner
				const c2 = this.cellToWorld(i, j + 1); // top-right corner
				const c3 = this.cellToWorld(i + 1, j); // bottom-left corner
				const c4 = this.cellToWorld(i + 1, j + 1); // bottom-right corner
				const o1 = lineMath.orientation(p1, p2, c1);
				const o2 = lineMath.orientation(p1, p2, c2);
				const o3 = lineMath.orientation(p1, p2, c3);
				const o4 = lineMath.orientation(p1, p2, c4);
				// now all corners must be on the same side of the line if the line doesn't intersect the cell (they must have the same non-zero orientation)
				if (o1 > 0 && o2 > 0 && o3 > 0 && o4 > 0) continue; // all corners are on the positive side
				if (o1 < 0 && o2 < 0 && o3 < 0 && o4 < 0) continue; // all corners are on the negative side
				// if we got here then the line segment intersects the current cell, we add its vertices
				this.cells[i][j].add([seg[0], seg[1][0]]);
				this.cells[i][j].add([seg[0], seg[1][1]]);
			}
	}

	getVerticesInArea(topLeft: WorldPoint, bottomRight: WorldPoint): Set<element_type> {
		assert(bottomRight.x >= topLeft.x && bottomRight.y >= topLeft.y);
		const coord1 = this.worldToCell(topLeft);
		const row1 = coord1[0];
		const col1 = coord1[1];
		const coord2 = this.worldToCell(bottomRight);
		const row2 = Math.min(this.cells.length - 1, coord2[0]);
		const col2 = Math.min(this.cells[0].length - 1, coord2[1]);

		const ret = new Set<element_type>();
		for (let i = row1; i <= row2; i++) {
			for (let j = col1; j <= col2; j++) {
				for (const el of this.cells[i][j]) {
					ret.add(el);
				}
			}
		}
		return ret;
	}

	clear(): void {
		for (const r of this.cells) for (const c of r) c.clear();
	}

	// ---------------------------------- PRIVATE AREA ----------------------------------

	private worldToCell(wp: WorldPoint): number[] {
		let row: number = (wp.y / this.grid.cellSize() - this.worldArea.topLeft().y) / this.relativeCellSize_;
		let col: number = (wp.x / this.grid.cellSize() - this.worldArea.topLeft().x) / this.relativeCellSize_;
		if (row < 0) row = 0;
		if (Math.floor(row) >= this.cells.length) row = this.cells.length - 1;
		if (col < 0) col = 0;
		if (Math.floor(col) >= this.cells[0].length) col = this.cells[0].length - 1;
		return [row, col];
	}

	private cellToWorld(row: number, col: number): WorldPoint {
		const x = (col * this.relativeCellSize_ + this.worldArea.topLeft().x) * this.grid.cellSize();
		const y = (row * this.relativeCellSize_ + this.worldArea.topLeft().y) * this.grid.cellSize();
		return new WorldPoint(x, y);
	}
}
