#include "Player.h"

void Player::endTurn() {
	turn_ = TURN_INACTIVE;
}

void Player::activateTurn(TurnType type) {
	turn_ = type;
	hasAction_ = false;
}

void Player::performAction(GridPoint point) {
	if (!isTurnActive() || hasAction_)
		return;
	hasAction_ = true;
	selectedPoint_ = point;
}

void Player::setAllowedVectors(std::vector<Arrow> vectors) {
	allowedVectors_ = vectors;
}
