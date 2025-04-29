#include "Player.h"

Player::Player(PlayerType type)
	: type_(type)
{

}

void Player::endTurn() {
	turn_ = TURN_INACTIVE;
}

void Player::activateTurn(TurnType type) {
	turn_ = type;
	hasAction_ = false;
}

void Player::confirmNextPoint() {
	if (!isTurnActive() || hasAction_)
		return;
	hasAction_ = true;
}

void Player::setAllowedVectors(std::vector<Arrow> vectors) {
	allowedVectors_ = vectors;
}
