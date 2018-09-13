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

	void addVertex(std::pair<int, int> index, WorldPoint const& v);
	std::set<std::pair<int, int>> getVerticesInArea(WorldPoint topLeft, WorldPoint bottomRight) const;

private:
	Track &track_;

	using element_type = std::pair<int, int>;
	using cell_type = std::set<element_type>;
	using row_type = std::vector<cell_type>;
	std::vector<row_type> cells;
	int relativeCellSize_;
};

#endif //__TRACK_PARTITION_H__
