#ifndef __GUI_EVENT_H__
#define __GUI_EVENT_H__

struct GUIEvent {
	enum EventType {
		INVALID,
		POINTER_ENTER,
		POINTER_LEAVE,
		POINTER_MOVE,
		TOUCH_BEGIN,
		TOUCH_END,
		TOUCH_DRAG,
		TOUCH_CLICK
	};
	EventType type = INVALID;	// type of event
	int x = 0;		// relative (to element's origin) x coordinate
	int y = 0;		// relative (to element's origin) y coordinate
	int dx = 0;		// delta (movement) on x axis
	int dy = 0;		// delta (movement) on y axis
};

#endif //__GUI_EVENT_H__
