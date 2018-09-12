#include "transform.h"
#include "Grid.h"
#include "WorldArea.h"
#include "Track.h"
#include "GUI/GUISystem.h"
#include "GUI/Button.h"
#include "color.h"

#include <SDL2/SDL.h>

#include <iostream>

const int windowW = 1280;
const int windowH = 720;
const int squareSize = 20;
float trackResolution = 2.f;

bool SIGNAL_QUIT = false;

Transform tr;
Grid grid(squareSize, windowW, windowH);
WorldArea warea(&grid, {1, 1}, {63, 35});
Track track(&grid, &warea, trackResolution);

GUISystem guiSystem;

GridPoint mousePoint{0, 0, 0};

void update(float dt) {

}

void render(SDL_Renderer *renderer) {
	grid.render(renderer);
	warea.render(renderer);
	track.render(renderer);

	// draw grid point closest to mouse:
	if (!track.isInDesignMode()) {
		ScreenPoint p = grid.gridToScreen(mousePoint);
		SDL_Rect rc {
			p.x - 3, p.y - 3,
			7, 7
		};
		Colors::GRID.set(renderer);
		SDL_RenderFillRect(renderer, &rc);
	}

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
			if (track.isInDesignMode()) {
				WorldPoint wp = ScreenPoint{ev.motion.x, ev.motion.y}.toWorld(tr);
				track.pointerTouch(ev.button.state == SDL_PRESSED, wp.x, wp.y);
			}
		}
	break;
	case SDL_MOUSEMOTION:
		if (isDragging) {
			float dx = ev.motion.xrel / tr.scale;
			float dy = ev.motion.yrel / tr.scale;
			tr.transX += dx;
			tr.transY += dy;
			grid.setTransform(tr);
		} else if (track.isInDesignMode()) {
			WorldPoint wp = ScreenPoint{ev.motion.x, ev.motion.y}.toWorld(tr);
			track.pointerMoved(wp.x, wp.y);
		}
		mousePoint = grid.screenToGrid({ev.motion.x, ev.motion.y});
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

		grid.setTransform(tr);
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
	Button* btn = new Button(30, 30, 120, 30);
	btn->setText("Draw Track");
	btn->setAction([&](Button* b) {
		if (track.isInDesignMode()) {
			track.enableDesignMode(false);
		} else {
			track.reset();
			track.enableDesignMode(true);
		}
	});
	guiSystem.addElement(std::unique_ptr<Button>(btn));
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
	Colors::BACKGROUND.set(renderer);
	SDL_RenderClear(renderer);

	initialize();

	do {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			handleSDLEvent(ev);
		}
		update(0.f);

		Colors::BACKGROUND.set(renderer);
		SDL_RenderClear(renderer);
		render(renderer);
		SDL_RenderPresent(renderer);
	} while (!SIGNAL_QUIT);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}