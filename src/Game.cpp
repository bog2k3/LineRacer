#include "Game.h"
#include "Track.h"
#include "Grid.h"
#include "Track.h"
#include "Player.h"
#include "lineMath.h"
#include "color.h"
#include "Painter.h"

#include <stdexcept>
#include <algorithm>

Game::Game(Track* track, float turnTimeLimit, unsigned targetLaps)
	: track_(track), turnTimeLimit_(turnTimeLimit), targetLaps_(targetLaps)
{
}

Game::~Game() {
	stop();
}

void Game::setState(GameState state) {
	state_ = state;
	onStateChange.trigger(state);
}

void Game::stop() {
	// TODO stop the game and notify all players
	// ...
}

void Game::update(float dt) {
	switch (state_) {
	case STATE_WAITING_PLAYERS:
		if (players_.size() == track_->getStartPositions().size())	// all positions have been filled, start automatically
			start();
		return;
	case STATE_STOPPED:
		return;
	case STATE_START_SELECTION:
	case STATE_PLAYING:
		turnTimer_ += dt;
		if (turnTimer_ >= turnTimeLimit_ || players_[currentPlayer_].player->actionReady()) {
			// perform action for player, then move on to the next turn
			processPlayerSelection();
			nextTurn();
		}
	}
}

void Game::draw(Viewport*) {
	if (state_ != STATE_START_SELECTION && state_ != STATE_PLAYING)
		return;
	static const Color* colors[] {
		&Colors::PLAYER1,
		&Colors::PLAYER2,
		&Colors::PLAYER3,
		&Colors::PLAYER4,
		&Colors::PLAYER5,
	};
	for (unsigned i=(currentPlayer_+1)%players_.size(), n=0; n<players_.size(); n++, i=(i+1)%players_.size()) {
		for (auto &a : players_[i].arrows) {
			ScreenPoint p1 = track_->grid()->gridToScreen(a.from);
			ScreenPoint p2 = track_->grid()->gridToScreen(a.to);
			Painter::paintArrow(p1, p2, 10 * track_->grid()->getTransform().scale, M_PI/6, *colors[players_[i].player->color()]);
		}
	}
}

bool Game::addPlayer(Player* player) {
	if (players_.size() < track_->getStartPositions().size()) {
		player->setColor(players_.size());
		player->setOffTrackData({false, {-1, -1}});
		players_.push_back(PlayerInfo{player});
		return true;
	} else
		return false;
}

void Game::start() {
	if (state_ != STATE_STOPPED && state_ != STATE_WAITING_PLAYERS)
		throw std::runtime_error("game already started!");
	startPosTaken_.assign(track_->getStartPositions().size(), false);
	setState(STATE_START_SELECTION);
	currentPlayer_ = -1;
	nextTurn();
}

void Game::reset() {
	if (state_ != STATE_STOPPED)
		stop();
	players_.clear();
	startPosTaken_.clear();
	currentPlayer_ = 0;
	turnTimer_ = 0;

	setState(STATE_WAITING_PLAYERS);
}

void Game::nextTurn() {
	if (players_.size() == 0)
		setState(STATE_STOPPED);
	if (state_ == STATE_WAITING_PLAYERS || state_ == STATE_STOPPED)
		return;
	if (currentPlayer_ >= 0) {
		players_[currentPlayer_].player->endTurn();
		if (checkWin())
			players_[currentPlayer_].player->activateTurn(Player::TURN_FINISHED);
	}
	while (++currentPlayer_ < players_.size() && players_[currentPlayer_].player->isFinished()) {
	}
	if (currentPlayer_ == players_.size()) {
		// all players took their turn for this round
		currentPlayer_ = 0;
		if (state_ == STATE_START_SELECTION) {
			setState(STATE_PLAYING);
		} else {
			while (currentPlayer_ < players_.size() && players_[currentPlayer_].player->isFinished())
				currentPlayer_++;
			if (currentPlayer_ == players_.size()) {
				// all players finished the game
				setState(STATE_STOPPED);
				return;
			}
		}
	}
	players_[currentPlayer_].player->activateTurn(state_ == STATE_START_SELECTION ? Player::TURN_SELECT_START : Player::TURN_MOVE);
	players_[currentPlayer_].player->setAllowedVectors(getPlayerVectors());
	onTurnAdvance.trigger();
}

bool Game::checkWin() {
	/*
		count how many laps a player did
		laps is initially 0
		when player crosses the start-line SEGMENT, his laps is incremented/decremented depending on direction
		when player is OUTSIDE of track and crosses the start-line INFINITE LINE, his laps is also incremented/decremented based on direction
			this is to avoid player cheating, exiting the track, going behind start-line and instantly winning when returning back and crossing it
	*/
	if (players_[currentPlayer_].arrows.size() < 2)
		return false;
	int cross = track_->checkStartLineCross(players_[currentPlayer_].arrows.back().from, players_[currentPlayer_].arrows.back().to, false);
	players_[currentPlayer_].startLineCrossCount += cross;
	if (cross > 0 && players_[currentPlayer_].startLineCrossCount > 0) {
		players_[currentPlayer_].laps++;
		if (players_[currentPlayer_].laps >= targetLaps_) {
			// player won
			return true;
		}
	}
	return false;
}

bool Game::validatePlayerMove(GridPoint move) {
	auto validMoves = getPlayerVectors();
	for (auto &a : validMoves)
		if (move == a.to)
			return true;
	return false;
}

std::vector<Arrow> Game::getPlayerVectors() {
	std::vector<Arrow> ret;
	if (state_ == STATE_START_SELECTION) {
		for (unsigned i=0; i<track_->getStartPositions().size(); i++)
			if (!startPosTaken_[i])
				ret.push_back(Arrow::fromPointAndDir(track_->getStartPositions()[i].position,
					track_->getStartPositions()[i].direction.first,
					track_->getStartPositions()[i].direction.second));
	} else if (state_ == STATE_PLAYING) {
		auto lastDir = players_[currentPlayer_].arrows.back().direction();
		GridPoint lastP = players_[currentPlayer_].arrows.back().to;
		Arrow center = Arrow::fromPointAndDir(lastP, lastDir.first, lastDir.second);
		for (int i=-1; i<=1; i++)
			for (int j=-1; j<=1; j++) {
				Arrow a {center.from, {center.to.x + j, center.to.y + i}};
				int maxLength = players_[currentPlayer_].isOffTrack ? 1 : 6;
				if (a.length() > maxLength)
					continue;
				if (!pathIsFree(a))
					continue;
				if (players_[currentPlayer_].isOffTrack) {
					// player was off-track, must check if the current arrow would get him back on
					// and if it does, only allow it if it re-enters the track behind his exit position
					WorldPoint arrowTipW = track_->grid()->gridToWorld(a.to);
					if (track_->pointInsidePolygon(arrowTipW, 0) && !track_->pointInsidePolygon(arrowTipW, 1)) {
						auto crossIndex = track_->computeCrossingIndex(a.from, a.to);
						if ((crossIndex.second - players_[currentPlayer_].trackCrossingIndex.second) * track_->polyDirection(crossIndex.first) > 0)
							continue;
					}
				}
				ret.push_back(a);
			}
	} else
		throw std::runtime_error("Invalid state");
	return ret;
}

Arrow Game::autoSelectPlayerVector() {
	if (!players_[currentPlayer_].arrows.size())
		throw std::runtime_error("Invalid state!");
	Arrow lastVector = players_[currentPlayer_].arrows.back();
	auto validMoves = getPlayerVectors();
	// try to keep the same vector as before if it's valid
	for (auto &a : validMoves)
		if (a.direction().first == lastVector.direction().first && a.direction().second == lastVector.direction().second && a.length() == lastVector.length())
			return a;
	// same vector could not be selected, try one with the same direction
	Arrow *sameDir[2] {nullptr, nullptr};
	int nSame=0;
	for (auto &a : validMoves)
		if (a.direction().first == lastVector.direction().first && a.direction().second == lastVector.direction().second)
			sameDir[nSame++] = &a;
	if (nSame > 0) {
		if (nSame == 1 || sameDir[0]->length() < sameDir[1]->length())	// choose the shorter (or the only) one
			return *sameDir[0];
		else
			return *sameDir[1];
	}
	// no same-direction vector, must steer left or right, choose randomly
	std::random_shuffle(validMoves.begin(), validMoves.end());
	return validMoves[0];
}

void Game::processPlayerSelection() {
	GridPoint nextPoint = players_[currentPlayer_].player->actionPoint();
	if (!validatePlayerMove(nextPoint)) {
		nextPoint = autoSelectPlayerVector().to;
	}
	// push the next point
	switch (state_) {
	case STATE_START_SELECTION:
		for (unsigned i=0; i<track_->getStartPositions().size(); i++) {
			GridPoint arrowTip { track_->getStartPositions()[i].position.x + track_->getStartPositions()[i].direction.first,
				track_->getStartPositions()[i].position.y + track_->getStartPositions()[i].direction.second };
			if (nextPoint == arrowTip) {
				// player chose position i
				Arrow a = Arrow::fromPointAndDir(track_->getStartPositions()[i].position,
					track_->getStartPositions()[i].direction.first,
					track_->getStartPositions()[i].direction.second);
				players_[currentPlayer_].arrows.push_back(a);
				players_[currentPlayer_].player->setLastArrow(a);
				startPosTaken_[i] = true;
				return;
			}
		}
		// if we got here, player's selection was none of the valid ones, kick him out
		players_[currentPlayer_].player->activateTurn(Player::TURN_FINISHED);
		break;
	case STATE_PLAYING: {
		Arrow a = {players_[currentPlayer_].arrows.back().to, nextPoint};
		players_[currentPlayer_].arrows.push_back(a);
		players_[currentPlayer_].player->setLastArrow(a);
		// check if player went off the track
		if (!track_->pointInsidePolygon(track_->grid()->gridToWorld(nextPoint), 0)
			|| track_->pointInsidePolygon(track_->grid()->gridToWorld(nextPoint), 1)) {
				if (!players_[currentPlayer_].isOffTrack) {
					// player is off track, set his speed to zero
					players_[currentPlayer_].isOffTrack = true;
					players_[currentPlayer_].trackCrossingIndex = track_->computeCrossingIndex(a.from, a.to);
					players_[currentPlayer_].player->setOffTrackData({true, players_[currentPlayer_].trackCrossingIndex});
					players_[currentPlayer_].arrows.push_back({nextPoint, nextPoint});
					players_[currentPlayer_].player->setLastArrow(players_[currentPlayer_].arrows.back());
				}
			} else if (players_[currentPlayer_].isOffTrack) {
				// player went back on track
				players_[currentPlayer_].isOffTrack = false;
				players_[currentPlayer_].player->setOffTrackData({false, {-1, -1}});
			}
		// check if arrow crosses the infinite start-line outside of the track
		WorldPoint intersect;
		int crossSign = track_->checkStartLineCross(a.from, a.to, true, &intersect);
		if (crossSign != 0) {
			if (!isPointOnTrack(intersect)) {
				players_[currentPlayer_].startLineCrossCount += crossSign;
			}
		}
	} break;
	default:
		throw std::runtime_error("what are you trying to do?");
	}
}

bool Game::pathIsFree(Arrow const& a) const {
	WorldPoint p1 = track_->grid()->gridToWorld(a.from);
	WorldPoint p2 = track_->grid()->gridToWorld(a.to);
	for (unsigned i=0; i<players_.size(); i++) {
		if (!players_[i].arrows.size())
			continue;
		if (a.from == players_[i].arrows.back().to)	// we ignore the previous arrow because it would yield a false positive
			continue;
		if (a.to == players_[i].arrows.back().to)	// arrow would end up on top of an occupied position
			return false;
		WorldPoint q = track_->grid()->gridToWorld(players_[i].arrows.back().to);
		if (lineMath::onSegment(p1, q, p2))	// arrow would cross another player's position
			return false;
	}
	if (track_->intersectionsCount(a.from, a.to) > 1) // if the arrow intersects the track in more than one place, disallow it
		return false;
	return true;
}

bool Game::isPointOnTrack(WorldPoint const& wp) const {
	return track_->pointInsidePolygon(wp, 0) && !track_->pointInsidePolygon(wp, 1);
}

bool Game::isPointOnTrack(GridPoint const& p) const {
	WorldPoint wp = track_->grid()->gridToWorld(p);
	return isPointOnTrack(wp);
}
