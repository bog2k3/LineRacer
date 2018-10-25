#ifndef __GUI_ELEMENT_H__
#define __GUI_ELEMENT_H__

struct GUIEvent;
class Viewport;

class GUIElement {
public:
	virtual ~GUIElement() {}
	GUIElement() {}

	virtual void draw(Viewport*) = 0;

	// override this to provide arbitrary element shape detection
	// this is only called if the pointer is within the control's area and by default the entire rectangular area is used as shape
	virtual bool containsPoint(int x, int y) { return true; }

	// the default implementation calls one of the onSomething... methods depending on event type
	virtual bool handleEvent(GUIEvent const& e);

	struct Area {
		int x, y;
		int w, h;
		bool containsPoint(int px, int py) {
			return px >= x && px < x+w && py >= y && py < y+h;
		}
	} area_ {0, 0, 10, 10};

protected:
	virtual bool onPointerEnter(int x, int y) { return false; } // return true if event was consumed, false if ignored
	virtual bool onPointerLeave(int x, int y) { return false; } // return true if event was consumed, false if ignored
	virtual bool onPointerMove(int x, int y, int dx, int dy) { return false; } // return true if event was consumed, false if ignored
	virtual bool onTouchBegin(int x, int y) { return true; } // return true if event was consumed, false if ignored
	virtual bool onTouchEnd(int x, int y) { return true; } // return true if event was consumed, false if ignored
	virtual bool onTouchDrag(int x, int y, int dx, int dy) { return true; } // return true if event was consumed, false if ignored
	virtual bool onClick(int x, int y) { return true; } // return true if event was consumed, false if ignored
};

#endif //__GUI_ELEMENT_H__
