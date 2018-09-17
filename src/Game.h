#ifndef __GAME_H__
#define __GAME_H__

#include "transform.h"

#include <vector>

class Track;
struct SDL_Renderer;
class Player;

class Game {
public:
	Game(Track* track, float turnTimeLimit);
	~Game();

	void update(float dt);
	void render(SDL_Renderer* r);

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
	Player* activePlayer() const { return currentPlayer_ >= 0 && currentPlayer_ < players_.size() ? players_[currentPlayer_] : nullptr; }

	bool pathIsFree(Arrow const& a); // returns true if the arrow doesn't intersect any player's position

private:
	Track* track_;
	float turnTimeLimit_;
	std::vector<Player*> players_;
	std::vector<std::vector<Arrow>> arrows_;
	std::vector<bool> startPosTaken_;
	GameState state_ = STATE_WAITING_PLAYERS;
	unsigned currentPlayer_ = 0;
	float turnTimer_ = 0.f;

	void nextTurn();
	bool checkWin();
	void processPlayerSelection();
	bool validatePlayerMove(GridPoint move);
	std::vector<Arrow> getPlayerVectors();
	Arrow autoSelectPlayerVector();
};

#endif //__GAME_H__
