#include "transform.h"
#include "Grid.h"
#include "WorldArea.h"
#include "Track.h"
#include "Game.h"
#include "Player.h"
#include "HumanController.h"
#include "NetworkController.h"
#include "Painter.h"
#include "color.h"
#include "GUI/MainMenu.h"
#include "GUI/SinglePlayerMenu.h"
#include "GUI/JoinMultiPlayerMenu.h"
#include "GUI/HostMultiPlayerMenu.h"
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/utils/bitFlags.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/Infrastructure.h>
#include <boglfw/input/SDLInput.h>
#include <boglfw/utils/drawable.h>

#include <SDL2/SDL.h>
#undef MB_RIGHT

#include <iostream>
#include <algorithm>

const int windowW = 1280;
const int windowH = 720;
const int squareSize = 20;
const float MIN_ZOOM = 0.4f;
const float MAX_ZOOM = 3.f;
const float trackResolution = 2.f;
const float TURN_TIME_LIMIT = 5.f; // seconds

bool SIGNAL_QUIT = false;

Transform tr;

Grid grid(squareSize, windowW, windowH);
//WorldArea warea(&grid, {1, 1}, {63, 35});
//Track track(&grid, &warea, trackResolution);
//Game game(&track, TURN_TIME_LIMIT, 1);
//HumanController hctrl(game, grid);
//NetworkController nctrl(game);
std::shared_ptr<MainMenu> mainMenu;
std::shared_ptr<SinglePlayerMenu> singlePlayerMenu;
std::shared_ptr<JoinMultiPlayerMenu> joinMultiPlayerMenu;
std::shared_ptr<HostMultiPlayerMenu> hostMultiPlayerMenu;
std::shared_ptr<GuiContainerElement> pActiveMenu;
GuiSystem* guiSystem = nullptr;

GridPoint mousePoint{0, 0, 0};

void switchMenu(std::shared_ptr<GuiContainerElement> const& pM);

void update(float dt) {
	//game.update(dt);
}

void drawMousePoint(Viewport*) {
	// draw grid point closest to mouse:
	/*if (!track.isInDesignMode()) {
		ScreenPoint p = grid.gridToScreen(mousePoint);
		int radius = std::max(1.f, 3 * tr.scale);
		Shape2D::get()->drawRectangleFilled({p.x - radius, p.y - radius}, 0, {2*radius+1, 2*radius+1}, Colors::MOUSE_POINT);
	}*/
}

void drawInfoTexts(Viewport*) {
	const int fontSz = 18;
	// draw game status
	/*std::string gameStatusText;
	switch (game.state()) {
		case Game::STATE_STOPPED:
			gameStatusText = "Stopped."; break;
		case Game::STATE_WAITING_PLAYERS:
			gameStatusText = "Waiting for players..."; break;
		case Game::STATE_START_SELECTION:
			gameStatusText = "Selecting start positions..."; break;
		case Game::STATE_PLAYING:
			gameStatusText = "Playing."; break;
	}
	auto gameStatus = "Game: " + gameStatusText;
	ViewportCoord gsCoord {50, 5, ViewportCoord::percent, ViewportCoord::top | ViewportCoord::left};
	auto textSz = GLText::get()->getTextRect(gameStatus, fontSz);
	gsCoord += {-textSz.x*0.5f, textSz.y*0.5f};
	GLText::get()->print(gameStatus, gsCoord, 0, fontSz, glm::vec3(0.2f, 0.4, 1.f));*/

	// draw bottom watermark text
	GLText::get()->print("Line Racer",
				{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
				0, fontSz, glm::vec3(0.2f, 0.4, 1.0f));
}

void handleKeyEvent(InputEvent &ev) {
	switch (ev.type) {
	case InputEvent::EV_KEY_DOWN:
		if (ev.key == SDLK_ESCAPE)
			SIGNAL_QUIT = true;
		break;
	case InputEvent::EV_KEY_UP:
		break;
	default:
		break;
	}
}

void handleMouseEvent(InputEvent &ev) {
	static bool isDragging = false;
	switch (ev.type) {
	case InputEvent::EV_MOUSE_DOWN:
	case InputEvent::EV_MOUSE_UP:
		if (ev.mouseButton == InputEvent::MB_RIGHT) {
			isDragging = ev.type == InputEvent::EV_MOUSE_DOWN;
		} else if (ev.mouseButton == InputEvent::MB_LEFT) {
			/*if (track.isInDesignMode()) {
				WorldPoint wp = ScreenPoint{(int)ev.x, (int)ev.y}.toWorld(tr);
				track.pointerTouch(ev.type == InputEvent::EV_MOUSE_DOWN, wp.x, wp.y);
			} else {
				hctrl.onPointerTouch(ev.type == InputEvent::EV_MOUSE_DOWN);
			}*/
		}
	break;
	case InputEvent::EV_MOUSE_MOVED:
		if (isDragging) {
			float dx = ev.dx / tr.scale;
			float dy = ev.dy / tr.scale;
			tr.transX += dx;
			tr.transY += dy;
			grid.setTransform(tr);
		} /*else if (track.isInDesignMode()) {
			WorldPoint wp = ScreenPoint{(int)ev.x, (int)ev.y}.toWorld(tr);
			track.pointerMoved(wp.x, wp.y);
		} else {
			hctrl.onPointerMoved(grid.screenToGrid({(int)ev.x, (int)ev.y}));
		}*/
		mousePoint = grid.screenToGrid({(int)ev.x, (int)ev.y});
	break;
	case InputEvent::EV_MOUSE_SCROLL: {
		// zoom view:
		float oldScale = tr.scale;
		if ((ev.dz > 0 && tr.scale < MAX_ZOOM) || (ev.dz < 0 && tr.scale > MIN_ZOOM))
			tr.scale *= (ev.dz > 0) ? 1.1f : (1.f / 1.1f);

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

void handleInputEvent(InputEvent &ev) {
	if (ev.isConsumed())
		return;
	guiSystem->handleInput(ev);
	if (ev.isConsumed())
		return;
	switch (ev.type) {
	case InputEvent::EV_KEY_DOWN:
	case InputEvent::EV_KEY_UP:
		handleKeyEvent(ev);
		break;
	case InputEvent::EV_MOUSE_DOWN:
	case InputEvent::EV_MOUSE_UP:
	case InputEvent::EV_MOUSE_MOVED:
	case InputEvent::EV_MOUSE_SCROLL:
		handleMouseEvent(ev);
		break;
	default:
		break;
	}
}

void joinMultiplayer() {

}

void hostMultiplayer() {

}

void initialize(SDL_Window* window) {
	// initialize input:
	SDLInput::initialize(window);
	SDLInput::onInputEvent.add(&handleInputEvent);

	// initialize logic:
	/*game.onTurnAdvance.add([&]() {
		hctrl.nextTurn();
		nctrl.nextTurn();
	});*/

	// initialize UI
	int wW, wH;
	SDL_GetWindowSize(window, &wW, &wH);
	glm::vec2 screenSize{wW, wH};
	mainMenu = std::make_shared<MainMenu>(screenSize);
	mainMenu->hide();
	guiSystem->addElement(mainMenu);
	mainMenu->onExit.add([&] {
		SIGNAL_QUIT = true;
	});
	mainMenu->onSinglePlayer.add([&] {
		switchMenu(singlePlayerMenu);
	});
	mainMenu->onJoinMulti.add([&] {
		switchMenu(joinMultiPlayerMenu);
	});
	mainMenu->onHostMulti.add([&] {
		switchMenu(hostMultiPlayerMenu);
	});

	singlePlayerMenu = std::make_shared<SinglePlayerMenu>(screenSize);
	singlePlayerMenu->hide();
	singlePlayerMenu->onBack.add([&] {
		switchMenu(mainMenu);
	});
	guiSystem->addElement(singlePlayerMenu);

	joinMultiPlayerMenu = std::make_shared<JoinMultiPlayerMenu>(screenSize);
	joinMultiPlayerMenu->hide();
	joinMultiPlayerMenu->onBack.add([&] {
		switchMenu(mainMenu);
	});
	guiSystem->addElement(joinMultiPlayerMenu);

	hostMultiPlayerMenu = std::make_shared<HostMultiPlayerMenu>(screenSize);
	hostMultiPlayerMenu->hide();
	hostMultiPlayerMenu->onBack.add([&] {
		switchMenu(mainMenu);
	});
	guiSystem->addElement(hostMultiPlayerMenu);
	/*auto btn = std::make_shared<Button>(glm::vec2{30, 30}, glm::vec2{120, 30}, "Draw Track");
	btn->onClick.add([&](Button* b) {
		if (track.isInDesignMode()) {
			track.enableDesignMode(false);
		} else {
			track.reset();
			track.enableDesignMode(true);
		}
	});
	guiSystem->addElement(btn);

	btn = std::make_shared<Button>(glm::vec2{30, 70}, glm::vec2{120, 30}, "Play!");
	btn->onClick.add([&](Button *b) {
		if (track.isReady()) {
			game.reset();
			Player *playerOne = new Player(Player::TYPE_HUMAN);	// major memory leak, just for debugging
			Player *playerTwo = new Player(Player::TYPE_HUMAN);	// major memory leak, just for debugging
			game.addPlayer(playerOne);
			game.addPlayer(playerTwo);
			game.start();
		}
	});
	guiSystem->addElement(btn);*/
}

void switchMenu(std::shared_ptr<GuiContainerElement> const& pM) {
	if (pActiveMenu)
		pActiveMenu->hide();
	if (pM)
		pM->show();
	pActiveMenu = pM;
}

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	auto argc = __argc;
	auto argv = __argv;
#else
int main(int argc, char** argv) {
#endif

	SDL_Window* window = nullptr;

	if (SDL_Init(SDL_INIT_EVENTS) < 0) {
		std::cerr << "Could not initialize SDL! SDL_Error: " << SDL_GetError() << "\n";
		return -1;
	}

	atexit(SDL_Quit);

	window = SDL_CreateWindow("Line Racer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!window) {
		std::cerr << "Could not create SDL window! SDL_Error: " << SDL_GetError() << "\n";
		return -1;
	}

	bool enableSuperSampling = false;

	bool gltInitResult = false;
	if (enableSuperSampling) {
		SSDescriptor ssDesc {SSDescriptor::SS_4X}; // 2x2 super samples
		gltInitResult = gltInitWithSDLSupersampled(window, ssDesc);
	} else {
		gltInitResult = gltInitWithSDL(window);
	}
	if (!gltInitResult) {
		std::cerr << "Could not initialize OpenGL!: " << SDL_GetError() << "\n";
		return -1;
	}

	Renderer boglfwRenderer(windowW, windowH);
	auto vp = std::make_unique<Viewport>(0, 0, windowW, windowH);
	vp->setBkColor((glm::vec3)Colors::BACKGROUND);

	guiSystem = new GuiSystem(vp.get(), {0, 0}, {windowW, windowH});

	std::vector<drawable> drawList {
		&grid,
		//&warea,
		//&track,
		//&game,
		&drawMousePoint,
		//&hctrl,
		//&nctrl,
		guiSystem,
		&drawInfoTexts,
	};
	vp->setDrawList(drawList);

	boglfwRenderer.addViewport("main", std::move(vp));

	initialize(window);
	switchMenu(mainMenu);

	while (SDLInput::checkInput() && !SIGNAL_QUIT) {
		update(0.f);

		gltBegin(Colors::BACKGROUND);
		boglfwRenderer.render();
		gltEnd();
	}

	delete guiSystem;
	guiSystem = nullptr;

	boglfwRenderer.unload();
	Infrastructure::shutDown();

	SDL_DestroyWindow(window);

	return 0;
}
