#include "transform.h"
#include "Grid.h"
#include "WorldArea.h"
#include "GUI/GUISystem.h"

#include <SDL2/SDL.h>

#include <iostream>

const int windowW = 1280;
const int windowH = 720;
const int squareSize = 20;

bool SIGNAL_QUIT = false;

Transform tr;
Grid grid(squareSize, windowW, windowH);
WorldArea warea(&grid, {1, 1}, {63, 35});

GUISystem guiSystem;

GridPoint mousePoint{0, 0, 0};

void update(float dt) {

}

void render(SDL_Renderer *renderer) {
	grid.render(renderer, tr);
	warea.render(renderer, tr);

	// draw grid point closest to mouse:
	ScreenPoint p = grid.gridToScreen(mousePoint, tr);
	SDL_Rect rc {
		p.x - 3, p.y - 3,
		7, 7
	};
	SDL_RenderFillRect(renderer, &rc);

	// draw user interface
	guiSystem.render(renderer);
}

void handleKeyEvent(SDL_KeyboardEvent &ev) {
	switch (ev.type) {
	case SDL_KEYDOWN:
		if (ev.keysym.sym == SDLK_ESCAPE)
			SIGNAL_QUIT = true;
		break;
	case SDL_KEYUP:
		break;
	}
}

void handleMouseEvent(SDL_Event &ev) {
	static bool isDragging = false;
	switch (ev.type) {
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		if (ev.button.button == 3) {
			isDragging = ev.button.state == SDL_PRESSED;
		} else if (ev.button.button == 1) {

		}
	break;
	case SDL_MOUSEMOTION:
		if (isDragging) {
			float dx = ev.motion.xrel / tr.scale;
			float dy = ev.motion.yrel / tr.scale;
			tr.transX += dx;
			tr.transY += dy;
		}
		mousePoint = grid.screenToGrid({ev.motion.x, ev.motion.y}, tr);
	break;
	case SDL_MOUSEWHEEL: {
		// zoom view:
		float oldScale = tr.scale;
		tr.scale *= (ev.wheel.y > 0) ? 1.1f : 0.9f;

		// now adjust the translation to keep it centered
		float oldFitW = windowW / oldScale;
		float newFitW = windowW / tr.scale;
		tr.transX += (newFitW - oldFitW) / 2;
		float oldFitH = windowH / oldScale;
		float newFitH = windowH / tr.scale;
		tr.transY += (newFitH - oldFitH) / 2;
	} break;
	default:
		break;
	}
}

void handleSDLEvent(SDL_Event &ev) {
	switch (ev.type) {
	case SDL_QUIT:
		SIGNAL_QUIT = true;
		return;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if (guiSystem.handleEvent(ev))
			return;	// event was consumed by UI
		handleKeyEvent(ev.key);
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEWHEEL:
		if (guiSystem.handleEvent(ev))
			return;	// event was consumed by UI
		handleMouseEvent(ev);
		break;
	default:
		break;
	}
}

void initialize() {
	// initialize UI
	//...
}

int WinMain() {

	SDL_Window* window = nullptr;
	SDL_Surface* screenSurf = nullptr;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Could not initialize SDL! SDL_Error: " << SDL_GetError() << "\n";
		return -1;
	}

	atexit(SDL_Quit);

	window = SDL_CreateWindow("Line Racer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH, SDL_WINDOW_SHOWN);
	if (!window) {
		std::cerr << "Could not create SDL window! SDL_Error: " << SDL_GetError() << "\n";
	}

	screenSurf = SDL_GetWindowSurface(window);

	SDL_FillRect(screenSurf, NULL, SDL_MapRGB( screenSurf->format, 0xFF, 0xF0, 0xE6 ) );
	SDL_UpdateWindowSurface( window );

	auto renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xF0, 0xE6, 0xFF);
	SDL_RenderClear(renderer);

	initialize();

	do {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			handleSDLEvent(ev);
		}
		update(0.f);

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xF0, 0xE6, 0xFF);
		SDL_RenderClear(renderer);
		render(renderer);
		SDL_RenderPresent(renderer);
	} while (!SIGNAL_QUIT);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}
