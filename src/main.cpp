#include "transform.h"
#include "Grid.h"
#include "WorldArea.h"
#include "Track.h"
#include "Game.h"
#include "Player.h"
#include "HumanController.h"
#include "Painter.h"
#include "GUI/GUISystem.h"
#include "GUI/Button.h"
#include "color.h"

#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/utils/bitFlags.h>

#include <boglfw/utils/DrawList.h>

#include <SDL2/SDL.h>
#include <asio.hpp>

#include <iostream>
#include <algorithm>

const int windowW = 1280;
const int windowH = 720;
const int squareSize = 20;
const float trackResolution = 2.f;
const float TURN_TIME_LIMIT = 5.f; // seconds

bool SIGNAL_QUIT = false;

Transform tr;
Grid grid(squareSize, windowW, windowH);
WorldArea warea(&grid, {1, 1}, {63, 35});
Track track(&grid, &warea, trackResolution);
Game game(&track, TURN_TIME_LIMIT, 1);
HumanController hctrl(game, grid);

GUISystem guiSystem;

GridPoint mousePoint{0, 0, 0};

void update(float dt) {
	game.update(dt);
}

void drawMousePoint(Viewport*) {
	// draw grid point closest to mouse:
	if (!track.isInDesignMode()) {
		ScreenPoint p = grid.gridToScreen(mousePoint);
		int radius = std::max(1.f, 3 * tr.scale);
		Shape2D::get()->drawRectangleFilled({p.x - radius, p.y - radius}, 0, {2*radius+1, 2*radius+1}, Colors::MOUSE_POINT);
	}
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
			} else {
				hctrl.onPointerTouch(ev.button.state == SDL_PRESSED);
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
		} else {
			hctrl.onPointerMoved(grid.screenToGrid({ev.motion.x, ev.motion.y}));
		}
		mousePoint = grid.screenToGrid({ev.motion.x, ev.motion.y});
	break;
	case SDL_MOUSEWHEEL: {
		// zoom view:
		float oldScale = tr.scale;
		tr.scale *= (ev.wheel.y > 0) ? 1.1f : (1.f / 1.1f);

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
	// initialize logic:
	game.onTurnAdvance.add([&]() {
		hctrl.nextTurn();
	});

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

	btn = new Button(30, 70, 120, 30);
	btn->setText("Play!");
	btn->setAction([&](Button *b) {
		if (track.isReady()) {
			game.reset();
			Player *playerOne = new Player(Player::TYPE_HUMAN);	// major memory leak, just for debugging
			Player *playerTwo = new Player(Player::TYPE_HUMAN);	// major memory leak, just for debugging
			game.addPlayer(playerOne);
			game.addPlayer(playerTwo);
			game.start();
		}
	});
	guiSystem.addElement(std::unique_ptr<Button>(btn));
}

#ifdef __WIN32__
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif
	SDL_Window* window = nullptr;
	SDL_Surface* screenSurf = nullptr;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Could not initialize SDL! SDL_Error: " << SDL_GetError() << "\n";
		return -1;
	}

	atexit(SDL_Quit);

	window = SDL_CreateWindow("Line Racer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!window) {
		std::cerr << "Could not create SDL window! SDL_Error: " << SDL_GetError() << "\n";
		return -1;
	}

	if (!gltInitWithSDL(window)) {
		std::cerr << "Could not initialize OpenGL!: " << SDL_GetError() << "\n";
		return -1;
	}

	Renderer boglfwRenderer(windowW, windowH);
	auto vp = std::make_unique<Viewport>(0, 0, windowW, windowH);
	vp->setBkColor((glm::vec3)Colors::BACKGROUND);
	boglfwRenderer.addViewport("main", std::move(vp));

	DrawList drawList;
	drawList.add(&grid);
	drawList.add(&warea);
	drawList.add(&track);
	drawList.add(&game);
	drawList.add(&drawMousePoint);
	drawList.add(&hctrl);
	drawList.add(&guiSystem);

	auto infoTexts = [&](Viewport*) {
		GLText::get()->print("Salut Lume!\n[Powered by boglfw]",
				{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
				0, 16, glm::vec3(0.2f, 0.4, 1.0f));
	};
	// drawList.add(&infoTexts);

	initialize();

	do {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			handleSDLEvent(ev);
		}
		update(0.f);

		gltBegin(Colors::BACKGROUND);
		boglfwRenderer.render(drawList);
		gltEnd();
	} while (!SIGNAL_QUIT);

	boglfwRenderer.unload();

	SDL_DestroyWindow(window);

	return 0;
}
