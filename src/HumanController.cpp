#include "HumanController.h"
#include "Game.h"
#include "Player.h"
#include "Grid.h"
#include "color.h"
#include "transform.h"
#include "Painter.h"

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
	} else {
		// touch released
		if (hasSelectedPoint_ && game_.activePlayer()->actionPoint() == selectedPoint_) {
			// this is a confirmation
			game_.activePlayer()->confirmNextPoint();
		} else {
			game_.activePlayer()->selectNextPoint(selectedPoint_);
			hasSelectedPoint_ = true;
		}
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

	// render the possible moves
	auto moves = game_.activePlayer()->validMoves();
	for (auto &m : moves) {
		if (game_.isPointOnTrack(m.to))
			Colors::VALID_MOVE.set(r);
		else
			Colors::OUT_TRACK_MOVE.set(r);
		ScreenPoint to = grid_.gridToScreen(m.to);
		const int POINT_RADIUS = std::max(1.f, 3 * grid_.getTransform().scale);
		SDL_Rect rc{to.x - POINT_RADIUS, to.y - POINT_RADIUS, 2*POINT_RADIUS+1, 2*POINT_RADIUS+1};
		if (hasSelectedPoint_ && selectedPoint_ == m.to)
			SDL_RenderFillRect(r, &rc);
		else
			SDL_RenderDrawRect(r, &rc);
	}
	if (game_.state() == Game::STATE_PLAYING) {
		// if no point selected yet, draw the previous vector, else draw the new vector to the selected point
		Arrow lastArrow = game_.activePlayer()->lastArrow();
		ScreenPoint from = grid_.gridToScreen(lastArrow.to);
		ScreenPoint to = hasSelectedPoint_ ? grid_.gridToScreen(selectedPoint_)
			: grid_.gridToScreen({lastArrow.to.x + lastArrow.direction().first, lastArrow.to.y + lastArrow.direction().second});
		Colors::UNCONFIRMED_ARROW.set(r);
		Painter::paintArrow(from, to, 10 * grid_.getTransform().scale, M_PI/6);
	}
}

void HumanController::nextTurn() {
	// reset stuff
	pointerDown_ = false;
	hasSelectedPoint_ = false;
}
