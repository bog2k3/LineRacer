#include "GUIEvent.h"
#include "GUIElement.h"

#include <stdexcept>

bool GUIElement::handleEvent(GUIEvent const &e) {
	switch (e.type) {
	case GUIEvent::POINTER_ENTER:
		return onPointerEnter(e.x, e.y);
	case GUIEvent::POINTER_LEAVE:
		return onPointerLeave(e.x, e.y);
	case GUIEvent::POINTER_MOVE:
		return onPointerMove(e.x, e.y, e.dx, e.dy);
	case GUIEvent::TOUCH_BEGIN:
		return onTouchBegin(e.x, e.y);
	case GUIEvent::TOUCH_END:
		return onTouchEnd(e.x, e.y);
	case GUIEvent::TOUCH_DRAG:
		return onTouchDrag(e.x, e.y, e.dx, e.dy);
	case GUIEvent::CLICK:
		return onClick(e.x, e.y);
	default:
		throw std::runtime_error("Unhandled event type");
	}
}
