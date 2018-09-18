#include "HumanController.h"
#include "Game.h"
#include "Player.h"
#include "Grid.h"
#include "color.h"
#include "transform.h"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

HumanController::HumanController(Game& game, Grid& grid)
	: game_(game), grid_(grid)
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
	if (!game_.activePlayer()
		|| game_.activePlayer()->type() != Player::TYPE_HUMAN
		|| !game_.activePlayer()->isTurnActive())
		return;

	// render the valid moves
	auto moves = game_.activePlayer()->validMoves();
	for (auto &m : moves) {
		ScreenPoint from = grid_.gridToScreen(m.from);
		ScreenPoint to = grid_.gridToScreen(m.to);
		const int POINT_RADIUS = 2;
		SDL_Rect rc{to.x - POINT_RADIUS, to.y - POINT_RADIUS, 2*POINT_RADIUS+1, 2*POINT_RADIUS+1};
		if (hasSelectedPoint_ && selectedPoint_ == m.to)
			Colors::VALID_MOVE_SELECTED.set(r);
		else
			Colors::VALID_MOVE.set(r);
		SDL_RenderDrawRect(r, &rc);
	}
	// if no point selected yet, draw the previous vector, else draw the new vector to the selected point

}

void HumanController::nextTurn() {
	// reset stuff
	pointerDown_ = false;
	hasSelectedPoint_ = false;
}
