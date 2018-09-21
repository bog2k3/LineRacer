#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "transform.h"

#include <vector>

class Player {
public:
	enum PlayerType {
		TYPE_HUMAN,
		TYPE_AI,
		TYPE_NETWORK
	};

	enum TurnType {
		TURN_INACTIVE,
		TURN_SELECT_START,
		TURN_MOVE,
		TURN_FINISHED
	};

	Player(PlayerType type);

	void setColor(int color) { color_ = color; }
	int color() const { return color_; }

	bool actionReady() const { return hasAction_; }	// returns true if the player has finished performing an action
	GridPoint actionPoint() const { return selectedPoint_; }	// gets the gridPoint where the last action happened
	void endTurn();	// clears the pending action and cancels waiting for the action, also takes control away from the player
	void activateTurn(TurnType type);	// called when the player's turn begins and he gets control to do his action
	void setAllowedVectors(std::vector<Arrow> vectors);
	void setOffTrackData(std::pair<bool, std::pair<int, float>> data) { offTrackData_ = data; }
	void setLastArrow(Arrow arrow) { lastArrow_ = arrow; }
	void selectNextPoint(GridPoint point) { selectedPoint_ = point; }
	void confirmNextPoint();

	PlayerType type() const { return type_; }
	bool isTurnActive() const { return turn_ != TURN_INACTIVE && turn_ != TURN_FINISHED; }
	bool isFinished() const { return turn_ == TURN_FINISHED; }
	std::vector<Arrow> validMoves() const { return allowedVectors_; }
	Arrow lastArrow() const { return lastArrow_; }
	std::pair<bool, std::pair<int, float>> offTrackData() const { return offTrackData_; }

private:
	PlayerType type_;
	int color_ = -1;
	TurnType turn_ = TURN_INACTIVE;
	bool hasAction_ = false;
	GridPoint selectedPoint_;
	std::vector<Arrow> allowedVectors_;
	Arrow lastArrow_;
	std::pair<bool, std::pair<int, float>> offTrackData_ {false, {-1, -1}};
};

#endif //__PLAYER_H__
