#include "TrackPartition.h"
#include "Track.h"
#include "WorldArea.h"
#include "Grid.h"

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

void TrackPartition::addVertex(std::pair<int, int> index, WorldPoint const& v) {
	int row = (v.y / track_.grid_->cellSize() - track_.worldArea_->topLeft().y) / relativeCellSize_;
	int col = (v.x / track_.grid_->cellSize() - track_.worldArea_->topLeft().x) / relativeCellSize_;

	assert(row >= 0 && (unsigned)row < cells.size());
	assert(col >= 0 && (unsigned)col < cells[0].size());

	cells[row][col].insert(index);
}

std::set<std::pair<int, int>> TrackPartition::getVerticesInArea(WorldPoint topLeft, WorldPoint bottomRight) const {
	int row1 = (topLeft.y / track_.grid_->cellSize() - track_.worldArea_->topLeft().y) / relativeCellSize_;
	int row2 = (bottomRight.y / track_.grid_->cellSize() - track_.worldArea_->topLeft().y) / relativeCellSize_;
	int col1 = (topLeft.x / track_.grid_->cellSize() - track_.worldArea_->topLeft().x) / relativeCellSize_;
	int col2 = (bottomRight.x / track_.grid_->cellSize() - track_.worldArea_->topLeft().x) / relativeCellSize_;

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
