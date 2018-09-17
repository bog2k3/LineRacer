#include "Game.h"
#include "Track.h"
#include "Grid.h"
#include "Track.h"
#include "Player.h"
#include "lineMath.h"
#include "color.h"

#include <SDL2/SDL_render.h>

#include <stdexcept>
#include <algorithm>

Game::Game(Track* track, float turnTimeLimit)
	: track_(track), turnTimeLimit_(turnTimeLimit)
{
	startPosTaken_.assign(track_->getStartPositions().size(), false);
}

Game::~Game() {
	stop();
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
		if (turnTimer_ >= turnTimeLimit_ || players_[currentPlayer_]->actionReady()) {
			// perform action for player, then move on to the next turn
			processPlayerSelection();
			nextTurn();
		}
	}
}

void Game::render(SDL_Renderer* r) {
	static const Color* colors[] {
		&Colors::PLAYER1,
		&Colors::PLAYER2,
		&Colors::PLAYER3,
		&Colors::PLAYER4,
		&Colors::PLAYER5,
	};
	for (unsigned i=0; i<players_.size(); i++) {
		colors[players_[i]->color()]->set(r);
		for (auto &a : arrows_[i]) {
			ScreenPoint p1 = track_->grid()->gridToScreen(a.from);
			ScreenPoint p2 = track_->grid()->gridToScreen(a.to);
			SDL_RenderDrawLine(r, p1.x, p1.y, p2.x, p2.y);
		}
	}
}

bool Game::addPlayer(Player* player) {
	if (players_.size() < track_->getStartPositions().size()) {
		player->setColor(players_.size());
		players_.push_back(player);
		return true;
	} else
		return false;
}

void Game::start() {
	state_ = STATE_START_SELECTION;
	currentPlayer_ = -1;
	nextTurn();
}

void Game::nextTurn() {
	if (players_.size() == 0)
		state_ = STATE_STOPPED;
	if (state_ == STATE_WAITING_PLAYERS || state_ == STATE_STOPPED)
		return;
	if (currentPlayer_ >= 0)
		players_[currentPlayer_]->endTurn();
	if (checkWin()) {
		players_[currentPlayer_]->activateTurn(Player::TURN_FINISHED);
	}
	if (++currentPlayer_ == players_.size()) {
		// all players took their turn for this round
		currentPlayer_ = 0;
		if (state_ == STATE_START_SELECTION) {
			state_ = STATE_PLAYING;
		} else {
			while (players_[currentPlayer_]->isFinished() && currentPlayer_ < players_.size())
				currentPlayer_++;
			if (currentPlayer_ == players_.size()) {
				// all players finished the game
				state_ = STATE_STOPPED;
				return;
			}
		}
	}
	players_[currentPlayer_]->activateTurn(state_ == STATE_START_SELECTION ? Player::TURN_SELECT_START : Player::TURN_MOVE);
	players_[currentPlayer_]->setAllowedVectors(getPlayerVectors());
}

bool Game::checkWin() {
	return false;
	// TODO...
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
		auto lastDir = arrows_[currentPlayer_].back().direction();
		GridPoint lastP = arrows_[currentPlayer_].back().to;
		Arrow center = Arrow::fromPointAndDir(lastP, lastDir.first, lastDir.second);
		for (int i=-1; i<=1; i++)
			for (int j=-1; j<=1; j++) {
				Arrow a {center.from, {center.to.x + j, center.to.y + i}};
				if (a.length() <= 6 && pathIsFree(a))
					ret.push_back(a);
			}
	} else
		throw std::runtime_error("Invalid state");
	return ret;
}

Arrow Game::autoSelectPlayerVector() {
	if (!arrows_[currentPlayer_].size())
		throw std::runtime_error("Invalid state!");
	Arrow lastVector = arrows_[currentPlayer_].back();
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
	GridPoint nextPoint = players_[currentPlayer_]->actionPoint();
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
				arrows_[currentPlayer_].push_back(Arrow::fromPointAndDir(track_->getStartPositions()[i].position,
					track_->getStartPositions()[i].direction.first,
					track_->getStartPositions()[i].direction.second));
				startPosTaken_[i] = true;
				return;
			}
		}
		// if we got here, player's selection was none of the valid ones, kick him out
		players_[currentPlayer_]->activateTurn(Player::TURN_FINISHED);
		break;
	case STATE_PLAYING:
		arrows_[currentPlayer_].push_back({arrows_[currentPlayer_].back().to, nextPoint});
		// check if player went off the track
		// ....
		break;
	default:
		throw std::runtime_error("what are you trying to do?");
	}
}

bool Game::pathIsFree(Arrow const& a) {
	WorldPoint p1 = track_->grid()->gridToWorld(a.from);
	WorldPoint p2 = track_->grid()->gridToWorld(a.to);
	for (unsigned i=0; i<players_.size(); i++) {
		if (!arrows_[i].size())
			continue;
		if (a.from == arrows_[i].back().to)	// we ignore the previous arrow because it would yield a false positive
			continue;
		if (a.to == arrows_[i].back().to)	// arrow would end up on top of an occupied position
			return false;
		WorldPoint q = track_->grid()->gridToWorld(arrows_[i].back().to);
		if (lineMath::onSegment(p1, q, p2))
			return false;
	}
	return true;
}
