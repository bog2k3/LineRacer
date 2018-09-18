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
	int rows = (track_.worldArea_->bottomRight().y - track_.worldArea_->topLeft().y) / relativeCellSize + 1;
	int cols = (track_.worldArea_->bottomRight().x - track_.worldArea_->topLeft().x) / relativeCellSize + 1;
	cells.assign(rows, row_type{});
	for (auto &r : cells)
		r.assign(cols, cell_type{});
}

std::pair<int, int> TrackPartition::worldToCell(WorldPoint const& wp) const {
	int row = (wp.y / track_.grid_->cellSize() - track_.worldArea_->topLeft().y) / relativeCellSize_;
	int col = (wp.x / track_.grid_->cellSize() - track_.worldArea_->topLeft().x) / relativeCellSize_;
	if (row < 0)
		row = 0;
	if ((unsigned)row >= cells.size())
		row = cells.size()-1;
	if (col < 0)
		col = 0;
	if ((unsigned)col >= cells[0].size())
		col = cells[0].size()-1;
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
	assert(bottomRight.x >= topLeft.x && bottomRight.y >= topLeft.y);
	auto coord1 = worldToCell(topLeft);
	int row1 = coord1.first;
	int col1 = coord1.second;
	auto coord2 = worldToCell(bottomRight);
	int row2 = std::min((int)cells.size()-1, coord2.first + 1);
	int col2 = std::min((int)cells[0].size()-1, coord2.second + 1);

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
