#ifndef __GAME_H__
#define __GAME_H__

#include "transform.h"
#include <boglfw/utils/Event.h>

#include <vector>

class Track;
class Player;
class Viewport;

class Game {
public:
	Game(Track* track, float turnTimeLimit, unsigned targetLaps);
	~Game();

	void update(float dt);
	void draw(Viewport*);

	void reset(); // resets the entire game, and removes all players
	bool addPlayer(Player* player);	// returns true if player was added and false if it couldn't be added (session is full)
	void start();
	void stop();

	enum GameState {
		STATE_WAITING_PLAYERS,
		STATE_START_SELECTION,
		STATE_PLAYING,
		STATE_STOPPED
	};

	GameState state() const { return state_; }
	Player* activePlayer() const { return currentPlayer_ >= 0 && (unsigned)currentPlayer_ < players_.size() ? players_[currentPlayer_].player : nullptr; }

	bool pathIsFree(Arrow const& a) const; // returns true if the arrow doesn't intersect any player's position
	bool isPointOnTrack(GridPoint const& p) const; // returns true if the point is within the track limits
	bool isPointOnTrack(WorldPoint const& p) const; // returns true if the point is within the track limits

	Track* track() const { return track_; }

	Event<void(GameState)> onStateChange;
	Event<void()> onTurnAdvance;

private:
	struct PlayerInfo {
		Player* player;
		std::vector<Arrow> arrows;
		int laps = 0;
		bool isOffTrack = false;
		std::pair<int, float> trackCrossingIndex {0, 0};	// first: contour index; second: track crossing index: point on track where he exited
		int startLineCrossCount = 0;	// counts how many times the player crossed the start line (sign depends on direction)

		PlayerInfo(Player* p) : player(p) {}
	};

	Track* track_;
	float turnTimeLimit_;
	std::vector<PlayerInfo> players_;
	std::vector<bool> startPosTaken_;
	GameState state_ = STATE_WAITING_PLAYERS;
	int currentPlayer_ = 0;
	float turnTimer_ = 0.f;
	unsigned targetLaps_ = 1;

	void setState(GameState state);
	void nextTurn();
	bool checkWin();
	void processPlayerSelection();
	bool validatePlayerMove(GridPoint move);
	std::vector<Arrow> getPlayerVectors();
	Arrow autoSelectPlayerVector();
};

#endif //__GAME_H__
