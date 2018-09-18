#include "color.h"

#include <SDL2/SDL_render.h>

void Color::set(SDL_Renderer* rend) const {
	SDL_SetRenderDrawColor(rend, r, g, b, a);
	if (a < 255)
		SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	else
		SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
}

namespace Colors {
	const Color RED {255, 0, 0};
	const Color RED_TR {255, 0, 0, 128};
	const Color GREEN {0, 255, 0};
	const Color GREEN_TR {0, 255, 0, 128};
	const Color BLUE {0, 0, 255};
	const Color BLUE_TR {0, 0, 255, 128};
	
	const Color BACKGROUND {0xFF, 0xF0, 0xE6};
	const Color GRID {190, 210, 255};
	const Color MOUSE_POINT {128, 165, 255};
	const Color TRACK {10, 35, 90};
	const Color TRACK_TRANSP {10, 35, 90, 64};
	const Color TRACK_SNAP {10, 145, 225};
	const Color STARTLINE {0, 128, 0};

	const Color OUT_TRACK_MOVE {210, 30, 0};
	const Color VALID_MOVE {0, 210, 110};

	const Color UNCONFIRMED_ARROW {40, 40, 40, 95};
	const Color PLAYER1 {0, 165, 235};
	const Color PLAYER2 {235, 28, 36};
	const Color PLAYER3 {34, 177, 76};
	const Color PLAYER4 {255, 201, 14};
	const Color PLAYER5 {163, 73, 164};

	const Color BUTTON_BORDER {5, 100, 110, 128};
	const Color BUTTON_FILL {80, 215, 235};
	const Color BUTTON_FILL_HOVER {135, 225, 240};
	const Color BUTTON_FILL_PRESSED {25, 145, 215};
};
