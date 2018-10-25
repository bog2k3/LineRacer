#ifndef __COLOR_H__
#define __COLOR_H__

#include <glm/vec4.hpp>

struct SDL_Renderer;

struct Color {
	int r, g, b;
	int a = 255;

	Color(int r, int g, int b) : r(r), g(g), b(b) {}
	Color(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}

	operator glm::vec4() const {
		return {r, g, b, a};
	}

	void set(SDL_Renderer* r) const;
};

namespace Colors {
	extern const Color RED;
	extern const Color RED_TR;
	extern const Color GREEN;
	extern const Color GREEN_TR;
	extern const Color BLUE;
	extern const Color BLUE_TR;

	extern const Color BACKGROUND;
	extern const Color GRID;
	extern const Color MOUSE_POINT;
	extern const Color TRACK;
	extern const Color TRACK_TRANSP;
	extern const Color TRACK_SNAP;
	extern const Color STARTLINE;

	extern const Color OUT_TRACK_MOVE;
	extern const Color VALID_MOVE;
	extern const Color INVALID_MOVE;

	extern const Color UNCONFIRMED_ARROW;
	extern const Color PLAYER1;
	extern const Color PLAYER2;
	extern const Color PLAYER3;
	extern const Color PLAYER4;
	extern const Color PLAYER5;

	extern const Color BUTTON_BORDER;
	extern const Color BUTTON_FILL;
	extern const Color BUTTON_FILL_HOVER;
	extern const Color BUTTON_FILL_PRESSED;
};

#endif //__COLOR_H__
