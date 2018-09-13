#include "TrackPartition.h"
#include "Track.h"
#include "WorldArea.h"
#include "Grid.h"
#include "lineMath.h"

#include <cassert>
#include <algorithm>

 TrackPartition::TrackPartition(Track &track, int relativeCellSize)
 	: track_(track), relativeCellSize_(relativeCellSize)
{
	int rows = (track_.worldArea_->bottomRight().y - track_.worldArea_->topLeft().y) * track_.grid_->cellSize() / relativeCellSize;
	int cols = (track_.worldArea_->bottomRight().x - track_.worldArea_->topLeft().x) * track_.grid_->cellSize() / relativeCellSize;
	cells.assign(rows, row_type{});
	for (auto &r : cells)
		r.assign(cols, cell_type{});
}

std::pair<int, int> TrackPartition::worldToCell(WorldPoint const& wp) const {
	int row = (wp.y / track_.grid_->cellSize() - track_.worldArea_->topLeft().y) / relativeCellSize_;
	int col = (wp.x / track_.grid_->cellSize() - track_.worldArea_->topLeft().x) / relativeCellSize_;
	return {row, col};
}

WorldPoint TrackPartition::cellToWorld(int row, int col) const {
	float x = (col * relativeCellSize_ + track_.worldArea_->topLeft().x) * track_.grid_->cellSize();
	float y = (row * relativeCellSize_ + track_.worldArea_->topLeft().y) * track_.grid_->cellSize();
	return {x, y};
}

void TrackPartition::addVertex(std::pair<int, int> index, WorldPoint const& v) {
	auto coord = worldToCell(v);
	int row = coord.first;
	int col = coord.second;

	assert(row >= 0 && (unsigned)row < cells.size());
	assert(col >= 0 && (unsigned)col < cells[0].size());

	cells[row][col].insert(index);
}

void TrackPartition::addSegment(std::pair<int, std::pair<int, int>> seg, WorldPoint const& p1, WorldPoint const& p2) {
	auto coord1 = worldToCell(p1);
	auto coord2 = worldToCell(p2);
	int row1 = std::min(coord1.first, coord2.first);
	int col1 = std::min(coord1.second, coord2.second);
	int row2 = std::max(coord1.first, coord2.first);
	int col2 = std::max(coord1.second, coord2.second);
	// we need to check every cell in the interval (r1,c1)->(r2,c2) for intersection with the line segment
	for (int i=row1; i<=row2; i++)
		for (int j=col1; j<=col2; j++) {
			auto c1 = cellToWorld(i, j);	// top-left corner
			auto c2 = cellToWorld(i, j+1);	// top-right corner
			auto c3 = cellToWorld(i+1, j);	// bottom-left corner
			auto c4 = cellToWorld(i+1, j+1);// bottom-right corner
			auto o1 = lineMath::orientation(p1, p2, c1);
			auto o2 = lineMath::orientation(p1, p2, c2);
			auto o3 = lineMath::orientation(p1, p2, c3);
			auto o4 = lineMath::orientation(p1, p2, c4);
			// now all corners must be on the same side of the line if the line doesn't intersect the cell (they must have the same non-zero orientation)
			if (o1 > 0 && o2 > 0 && o3 > 0 && o4 > 0)
				continue; // all corners are on the positive side
			if (o1 < 0 && o2 < 0 && o3 < 0 && o4 < 0)
				continue; // all corners are on the negative side
			// if we got here then the line segment intersects the current cell, we add its vertices
			cells[i][j].insert({seg.first, seg.second.first});
			cells[i][j].insert({seg.first, seg.second.second});
		}
}

std::set<std::pair<int, int>> TrackPartition::getVerticesInArea(WorldPoint topLeft, WorldPoint bottomRight) const {
	auto coord1 = worldToCell(topLeft);
	int row1 = coord1.first;
	int col1 = coord1.second;
	auto coord2 = worldToCell(bottomRight);
	int row2 = coord2.first;
	int col2 = coord2.second;

	assert(row2 >= row1 && col2 >= col1);

	if (row1 < 0) row1 = 0;
	if ((unsigned)row2 >= cells.size())
		row2 = cells.size() - 1;
	if (col1 < 0) col1 = 0;
	if ((unsigned)col2 >= cells.size())
		col2 = cells.size() -1 ;

	std::set<std::pair<int, int>> ret;
	for (int i=row1; i<=row2; i++)
		for (int j=col1; j<=col2; j++)
			std::copy(cells[i][j].begin(), cells[i][j].end(), std::inserter(ret, ret.end()));
	return ret;
}

void TrackPartition::clear() {
	for (auto &r : cells)
		for (auto &c : r)
			c.clear();
}
