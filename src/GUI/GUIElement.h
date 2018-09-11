#ifndef __GUI_ELEMENT_H__
#define __GUI_ELEMENT_H__

struct SDL_Renderer;
struct GUIEvent;

class GUIElement {
public:
	virtual ~GUIElement() {}
	GUIElement() {}

	// the default implementation calls one of the onSomething... methods depending on event type
	virtual bool handleEvent(GUIEvent const& e);

	virtual void render(SDL_Renderer* r) = 0;

	virtual bool onPointerEnter(int x, int y) { return false; } // return true if event was consumed, false if ignored
	virtual bool onPointerLeave(int x, int y) { return false; } // return true if event was consumed, false if ignored
	virtual bool onPointerMove(int x, int y, int dx, int dy) { return false; } // return true if event was consumed, false if ignored
	virtual bool onTouchBegin(int x, int y) { return false; } // return true if event was consumed, false if ignored
	virtual bool onTouchEnd(int x, int y) { return false; } // return true if event was consumed, false if ignored
	virtual bool onTouchDrag(int x, int y, int dx, int dy) { return false; } // return true if event was consumed, false if ignored
	virtual bool onTouchClick(int x, int y) { return false; } // return true if event was consumed, false if ignored

	struct Area {
		int x, y;
		int w, h;
		bool containsPoint(int px, int py) {
			return px >= x && px < x+w && py >= y && py < y+h;
		}
	} area {0, 0, 10, 10};
};

#endif //__GUI_ELEMENT_H__
