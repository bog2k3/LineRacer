#ifndef __HUMAN_CONTROLLER_H__
#define __HUMAN_CONTROLLER_H__

#include "transform.h"

class Game;
struct SDL_Renderer;

// controller for local human player
// handles local input events and translates them into Player actions
class HumanController {
public:
	HumanController(Game& game);
	~HumanController() {}

	void onPointerMoved(GridPoint where);
	void onPointerTouch(bool pressed);
	// this is called by the game when a new turn starts
	void nextTurn();

	void render(SDL_Renderer* r);

private:
	Game& game_;
	bool pointerDown_ = false;
	GridPoint selectedPoint_;
	GridPoint hoverPoint_;
	bool hasSelectedPoint_ = false;

	bool isPointValid(GridPoint const& p);
};

#endif //__HUMAN_CONTROLLER_H__
