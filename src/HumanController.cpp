#include "HumanController.h"
#include "Game.h"
#include "Player.h"
#include "Grid.h"
#include "color.h"
#include "transform.h"
#include "Painter.h"
#include "Track.h"
#include "lineMath.h"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#include <algorithm>

HumanController::HumanController(Game& game, Grid& grid)
	: game_(game), grid_(grid)
{
}

void HumanController::onPointerMoved(GridPoint where) {
	hoverPoint_ = where;
	if (pointerDown_ && isPointSelectable(where)) {
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

	if (!isPointSelectable(hoverPoint_))
		return;

	if (pressed) {
		selectedPoint_ = hoverPoint_;
	} else {
		// touch released
		if (hasSelectedPoint_ && isValidMove(selectedPoint_) && game_.activePlayer()->actionPoint() == selectedPoint_) {
			// this is a confirmation
			game_.activePlayer()->confirmNextPoint();
		} else {
			game_.activePlayer()->selectNextPoint(selectedPoint_);
			hasSelectedPoint_ = true;
		}
	}
}

bool HumanController::isValidMove(GridPoint const& p) {
	if (!game_.activePlayer()
		|| game_.activePlayer()->type() != Player::TYPE_HUMAN
		|| !game_.activePlayer()->isTurnActive())
		return false;

	for (auto &a : game_.activePlayer()->validMoves())
		if (a.to == p)
			return true;
	return false;
}

bool HumanController::isPointSelectable(GridPoint const& p) {
	for (int i=0; i<9; i++)
		if (p == possibleMoves_[i].p)
			return true;
	return false;
}

void HumanController::render(SDL_Renderer* r) {
	if (!game_.activePlayer()
		|| game_.activePlayer()->type() != Player::TYPE_HUMAN
		|| !game_.activePlayer()->isTurnActive())
		return;

	// render the possible moves
	for (auto &m : possibleMoves_) {
		if (m.isValid) {
			if (m.isWithinTrack)
				Colors::VALID_MOVE.set(r);
			else
				Colors::OUT_TRACK_MOVE.set(r);
		} else
			Colors::INVALID_MOVE.set(r);
		ScreenPoint to = grid_.gridToScreen(m.p);
		const int POINT_RADIUS = std::max(1.f, 3 * grid_.getTransform().scale);
		if (m.isValid) {
			SDL_Rect rc{to.x - POINT_RADIUS, to.y - POINT_RADIUS, 2*POINT_RADIUS+1, 2*POINT_RADIUS+1};
			if (hasSelectedPoint_ && selectedPoint_ == m.p)
				SDL_RenderFillRect(r, &rc);
			else
				SDL_RenderDrawRect(r, &rc);
		} else {
			// mark the invalid point with an X
			SDL_RenderDrawLine(r, to.x - POINT_RADIUS, to.y - POINT_RADIUS, to.x + POINT_RADIUS, to.y + POINT_RADIUS);
			SDL_RenderDrawLine(r, to.x - POINT_RADIUS, to.y + POINT_RADIUS, to.x + POINT_RADIUS, to.y - POINT_RADIUS);
		}
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
	// if player is off-track, highlight the contour area where he can re-enter
	if (game_.activePlayer()->offTrackData().first) {
		int exitPolygon = game_.activePlayer()->offTrackData().second.first;
		int exitSegIndex = floor(game_.activePlayer()->offTrackData().second.second);
		float exitSegWeight = game_.activePlayer()->offTrackData().second.second - exitSegIndex;
		float requiredNegativeDistance = grid_.cellSize() * 4;
		float requiredPositiveDistance = grid_.cellSize() * 2;
		// draw the positive contour:
		float positiveDist = 0;
		Colors::RED.set(r);
		for (int i=0; positiveDist < requiredPositiveDistance && i < game_.track()->polyLength(exitPolygon); i++) {
			float drawLength = 1.f;
			if (i == 0) {
				// this is the exit segment, we draw only a fraction of it
				drawLength -= exitSegWeight;
			}
			int i1 = (exitSegIndex + i) % game_.track()->polyLength(exitPolygon);
			int i2 = (exitSegIndex + i + 1) % game_.track()->polyLength(exitPolygon);
			WorldPoint wp1 = game_.track()->polyVertex(exitPolygon, i1);
			WorldPoint wp2 = game_.track()->polyVertex(exitPolygon, i2);
			positiveDist += lineMath::distance(wp1, wp2);
			ScreenPoint sp1 = wp1.toScreen(game_.track()->grid()->getTransform());
			ScreenPoint sp2 = wp2.toScreen(game_.track()->grid()->getTransform());
			SDL_RenderDrawLine(r, sp1.x, sp1.y, sp2.x, sp2.y);
		}
		// draw the negative contour:
		float negativeDist = 0;
		Colors::GREEN.set(r);
		for (int i=0; negativeDist< requiredNegativeDistance && i < game_.track()->polyLength(exitPolygon); i++) {
			float drawLength = 1.f;
			if (i == 0) {
				// this is the exit segment, we draw only a fraction of it
				drawLength = exitSegWeight;
			}
			int i1 = (exitSegIndex + game_.track()->polyLength(exitPolygon) - i - 1) % game_.track()->polyLength(exitPolygon);
			int i2 = (exitSegIndex + game_.track()->polyLength(exitPolygon) - i) % game_.track()->polyLength(exitPolygon);
			WorldPoint wp1 = game_.track()->polyVertex(exitPolygon, i1);
			WorldPoint wp2 = game_.track()->polyVertex(exitPolygon, i2);
			negativeDist += lineMath::distance(wp1, wp2);
			ScreenPoint sp1 = wp1.toScreen(game_.track()->grid()->getTransform());
			ScreenPoint sp2 = wp2.toScreen(game_.track()->grid()->getTransform());
			SDL_RenderDrawLine(r, sp1.x, sp1.y, sp2.x, sp2.y);
		}
	}
}

void HumanController::nextTurn() {
	// reset stuff
	pointerDown_ = false;
	hasSelectedPoint_ = false;
	// compute player's possible moves:
	auto validMoves = game_.activePlayer()->validMoves();
	possibleMoves_.clear();
	if (game_.state() == Game::STATE_START_SELECTION) {
		for (auto &a : validMoves)
			possibleMoves_.push_back({ a.to, true, true });
		return;
	}
	auto arrow = game_.activePlayer()->lastArrow();
	GridPoint topLeft {arrow.to.x + arrow.direction().first - 1, arrow.to.y + arrow.direction().second - 1};
	for (int i=0; i<9; i++) {
		int x = topLeft.x + (i/3);
		int y = topLeft.y + (i%3);
		possiblePoint pt;
		pt.p = {x, y};
		pt.isValid = std::find_if(validMoves.begin(), validMoves.end(), [this, &pt] (auto &a) {
			return a.to == pt.p;
		}) != validMoves.end();
		pt.isWithinTrack = game_.isPointOnTrack(pt.p);
		possibleMoves_.push_back(pt);
	}
}
