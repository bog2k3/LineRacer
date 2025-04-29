struct Transform {
	float scale = 1;	// screen to world unit ratio
	float transX = 0;	// world-space offset
	float transY = 0;	// workd-space offset
};

struct ScreenPoint;

struct WorldPoint {
	float x = 0;
	float y = 0;

	ScreenPoint toScreen(Transform const& tr);

	float distanceTo(WorldPoint const& p) {
		float xdif = x - p.x;
		float ydif = y - p.y;
		return sqrt(xdif*xdif + ydif*ydif);
	}

	bool operator==(WorldPoint const& p) const {
		return x==p.x && y==p.y;
	}
};

struct ScreenPoint {
	int x=0;
	int y=0;

	WorldPoint toWorld(Transform const& tr);

	operator glm::vec2() const {
		return {x, y};
	}
};

struct GridPoint {
	int x=0;			// x coordinate on grid
	int y=0;			// y coordinate on grid
	float distance=0;	// distance from actual pixel to the grid location, relative to grid size [0.0 .. sqrt(2)/2]

	GridPoint() = default;
	GridPoint(int x, int y) : x(x), y(y), distance(0) {}
	GridPoint(int x, int y, float dist) : x(x), y(y), distance(dist) {}

	bool operator==(GridPoint const& p) const {
		return x==p.x && y==p.y;
	}
};

struct Arrow {
	GridPoint from;
	GridPoint to;

	int length() const;
	std::pair<int, int> direction() const;

	static Arrow fromPointAndDir(GridPoint p, int dirX, int dirY) {
		return Arrow{p, {p.x + dirX, p.y + dirY}};
	}
};

template<class P, typename T1 = decltype(P::x), typename T2 = decltype(P::y)>
P operator+ (P const& p1, P const& p2) {
	return P{p1.x + p2.x, p1.y + p2.y};
}

template<class P, typename T1 = decltype(P::x), typename T2 = decltype(P::y)>
P operator- (P const& p1, P const& p2) {
	return P{p1.x - p2.x, p1.y - p2.y};
}