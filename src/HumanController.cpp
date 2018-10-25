#include "HumanController.h"
#include "Game.h"
#include "Player.h"
#include "Grid.h"
#include "color.h"
#include "transform.h"
#include "Painter.h"
#include "Track.h"
#include "lineMath.h"

#include <boglfw/renderOpenGL/Shape2D.h>

#include <algorithm>

HumanController::HumanController(Game& game, Grid& grid)
	: game_(game), grid_(grid)
{
}

void HumanController::onPointerMoved(GridPoint where) {
	if (!game_.activePlayer()
		|| game_.activePlayer()->type() != Player::TYPE_HUMAN
		|| !game_.activePlayer()->isTurnActive())
		return;

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

void HumanController::draw(Viewport*) {
	if (!game_.activePlayer()
		|| game_.activePlayer()->type() != Player::TYPE_HUMAN
		|| !game_.activePlayer()->isTurnActive())
		return;

	// render the possible moves
	for (auto &m : possibleMoves_) {
		const Color* color = &Colors::INVALID_MOVE;
		if (m.isValid) {
			if (m.isWithinTrack)
				color = &Colors::VALID_MOVE;
			else
				color = &Colors::OUT_TRACK_MOVE;
		}
		ScreenPoint to = grid_.gridToScreen(m.p);
		const int POINT_RADIUS = std::max(1.f, 3 * grid_.getTransform().scale);
		if (m.isValid) {
			//SDL_Rect rc{to.x - POINT_RADIUS, to.y - POINT_RADIUS, 2*POINT_RADIUS+1, 2*POINT_RADIUS+1};
			glm::vec2 pos {to.x - POINT_RADIUS, to.y - POINT_RADIUS};
			glm::vec2 size {2*POINT_RADIUS+1, 2*POINT_RADIUS+1};
			if (hasSelectedPoint_ && selectedPoint_ == m.p)
				Shape2D::get()->drawRectangleFilled(pos, 0, size, *color);
			else
				Shape2D::get()->drawRectangle(pos, 0, size, *color);
		} else {
			// mark the invalid point with an X
			Shape2D::get()->drawLine({to.x - POINT_RADIUS, to.y - POINT_RADIUS}, {to.x + POINT_RADIUS, to.y + POINT_RADIUS}, 0, *color);
			Shape2D::get()->drawLine({to.x - POINT_RADIUS, to.y + POINT_RADIUS}, {to.x + POINT_RADIUS, to.y - POINT_RADIUS}, 0, *color);
		}
	}
	if (game_.state() == Game::STATE_PLAYING) {
		// if no point selected yet, draw the previous vector, else draw the new vector to the selected point
		Arrow lastArrow = game_.activePlayer()->lastArrow();
		ScreenPoint from = grid_.gridToScreen(lastArrow.to);
		ScreenPoint to = hasSelectedPoint_ ? grid_.gridToScreen(selectedPoint_)
			: grid_.gridToScreen({lastArrow.to.x + lastArrow.direction().first, lastArrow.to.y + lastArrow.direction().second});
		Painter::paintArrow(from, to, 10 * grid_.getTransform().scale, M_PI/6, Colors::UNCONFIRMED_ARROW);
	}
	// if player is off-track, highlight the contour area where he can re-enter
	// This must take into account the polygon winding direction (CW or CCW) and the start-line direction as well
	if (game_.activePlayer()->offTrackData().first) {
		int exitPolygon = game_.activePlayer()->offTrackData().second.first;
		int exitSegIndex = floor(game_.activePlayer()->offTrackData().second.second);
		float exitSegWeight = game_.activePlayer()->offTrackData().second.second - exitSegIndex;
		float requiredRedDistance = grid_.cellSize() * 10;
		float requiredGreenDistance = grid_.cellSize() * 7;
		int reentryDirection = -game_.track()->polyDirection(exitPolygon); // reentry direction depends on polygon direction vs startLine direction

		for (int sign = +1; sign >= -1; sign -= 2) {
			bool greenPart = sign == reentryDirection;
			float distance = 0;
			float requiredDist = greenPart ? requiredGreenDistance : requiredRedDistance;
			Color c = greenPart > 0 ? Colors::GREEN : Colors::RED;
			for (int i=0; distance < requiredDist && i < game_.track()->polyLength(exitPolygon)/2; i++) {
				c.a = 255 * (1.f - distance / requiredDist);
				float drawLength = 1.f;
				if (i == 0) {
					// this is the exit segment, we draw only a fraction of it
					drawLength = sign > 0 ? 1.f - exitSegWeight : exitSegWeight;
					// when drawLength > 0, we draw from 0.0 to drawLength
					// when drawLength < 0, we draw from 1.0+drawLength to 1.0
				}
				int offs = sign < 0 ? 1 : 0;
				int i1 = (exitSegIndex + game_.track()->polyLength(exitPolygon) + offs + i * sign) % game_.track()->polyLength(exitPolygon);
				int i2 = (exitSegIndex + game_.track()->polyLength(exitPolygon) + offs + (i + 1)*sign) % game_.track()->polyLength(exitPolygon);
				WorldPoint wp1 = game_.track()->polyVertex(exitPolygon, i1);
				WorldPoint wp2 = game_.track()->polyVertex(exitPolygon, i2);
				// adjust if drawing part of the segment:
				if (drawLength != 1.f) {
					wp1.x = wp2.x + (wp1.x-wp2.x) * drawLength;
					wp1.y = wp2.y + (wp1.y-wp2.y) * drawLength;
				}
				distance += lineMath::distance(wp1, wp2);
				ScreenPoint sp1 = wp1.toScreen(game_.track()->grid()->getTransform());
				ScreenPoint sp2 = wp2.toScreen(game_.track()->grid()->getTransform());
				Shape2D::get()->drawLine(sp1, sp2, 0, c);
			}
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
