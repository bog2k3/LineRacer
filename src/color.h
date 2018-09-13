#ifndef __COLOR_H__
#define __COLOR_H__

struct SDL_Renderer;

struct Color {
	int r, g, b;
	int a = 255;

	Color(int r, int g, int b) : r(r), g(g), b(b) {}
	Color(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}

	void set(SDL_Renderer* r) const;
};

namespace Colors {
	static const Color BACKGROUND {0xFF, 0xF0, 0xE6};
	static const Color GRID {190, 210, 255};
	static const Color TRACK {10, 35, 90};
	static const Color TRACK_TRANSP {10, 35, 90, 64};
	static const Color TRACK_SNAP {10, 145, 225};
	static const Color BUTTON_BORDER {5, 100, 110, 128};
	static const Color BUTTON_FILL {80, 215, 235};
	static const Color BUTTON_FILL_HOVER {135, 225, 240};
	static const Color BUTTON_FILL_PRESSED {25, 145, 215};
};

#endif //__COLOR_H__
