#include "HumanController.h"
#include "Game.h"
#include "Player.h"
#include "transform.h"

HumanController::HumanController(Game& game)
	: game_(game)
{
}

void HumanController::onPointerMoved(GridPoint where) {
	hoverPoint_ = where;
	if (pointerDown_ && isPointValid(where)) {
		selectedPoint_ = hoverPoint_;
		hasSelectedPoint_ = true;
	}
}

void HumanController::onPointerTouch(bool pressed) {
	if (pressed == pointerDown_)
		return;
	pointerDown_ = pressed;

	if (!game_.activePlayer()
		|| game_.activePlayer()->type() != Player::TYPE_HUMAN
		|| !game_.activePlayer()->isTurnActive())
		return;

	if (!isPointValid(hoverPoint_))
		return;

	if (pressed) {
		selectedPoint_ = hoverPoint_;
		hasSelectedPoint_ = true;
	} else {
		// touch released
		if (game_.activePlayer()->actionPoint() == selectedPoint_) {
			// this is a confirmation
			game_.activePlayer()->confirmNextPoint();
		} else
			game_.activePlayer()->selectNextPoint(selectedPoint_);
	}
}

bool HumanController::isPointValid(GridPoint const& p) {
	if (!game_.activePlayer()
		|| game_.activePlayer()->type() != Player::TYPE_HUMAN
		|| !game_.activePlayer()->isTurnActive())
		return false;

	for (auto &a : game_.activePlayer()->validMoves())
		if (a.to == p)
			return true;
	return false;
}

void HumanController::render(SDL_Renderer* r) {
	// render the valid moves
}

void HumanController::nextTurn() {
	// reset stuff
	pointerDown_ = false;
	hasSelectedPoint_ = false;
}
