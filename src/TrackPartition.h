#ifndef __TRACK_PARTITION_H__
#define __TRACK_PARTITION_H__

#include <vector>
#include <set>

class Track;
struct WorldPoint;

class TrackPartition {
public:
	// relativeCellSize: how many grid cells is one partition cell wide and high?
	TrackPartition(Track &track, int relativeCellSize);

	// adds a vertex into space partitioning structure
	void addVertex(std::pair<int, int> index, WorldPoint const& v);
	// adds a line segment into the partitioning structure -> its vertices will be added to all cells that the segment overlaps
	void addSegment(std::pair<int, std::pair<int, int>> seg, WorldPoint const& p1, WorldPoint const& p2);
	std::set<std::pair<int, int>> getVerticesInArea(WorldPoint topLeft, WorldPoint bottomRight) const;

	void clear();

private:
	Track &track_;

	using element_type = std::pair<int, int>;
	using cell_type = std::set<element_type>;
	using row_type = std::vector<cell_type>;
	std::vector<row_type> cells;
	int relativeCellSize_;

	std::pair<int, int> worldToCell(WorldPoint const& wp) const;
	WorldPoint cellToWorld(int row, int col) const;
};

#endif //__TRACK_PARTITION_H__
