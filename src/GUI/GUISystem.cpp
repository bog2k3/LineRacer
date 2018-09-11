#include "GUISystem.h"
#include "GUIEvent.h"

#include <SDL2/SDL_events.h>

#include <cassert>
#include <stdexcept>

int GUISystem::addElement(std::unique_ptr<GUIElement> &&e) {
	int i = 0;
	while (i < elements_.size() && elements_[i].get())
		i++;
	if (i < elements_.size())
		elements_[i] = std::move(e);
	else
		elements_.push_back(std::move(e));
	return i;
}

void GUISystem::removeElement(int elementHandle) {
	assert(elementHandle >= 0 && elementHandle < elements_.size());
	assert(elements_[elementHandle].get());
	elements_[elementHandle].reset();
}

GUIElement* GUISystem::getElement(int elementHandle) {
	assert(elementHandle >= 0 && elementHandle < elements_.size());
	return elements_[elementHandle].get();
}

void GUISystem::render(SDL_Renderer* r) {
	for (auto &e : elements_) {
		if (e.get())
			e->render(r);
	}
}

bool GUISystem::translateEvent(SDL_Event const& e, GUIEvent &out) {
	switch(e.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	case SDL_TEXTINPUT:
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (e.button.button == 1) {
			out.type = GUIEvent::TOUCH_BEGIN;
			out.x = e.button.x;
			out.y = e.button.y;
			return true;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (e.button.button == 1) {
			out.type = GUIEvent::TOUCH_END;
			out.x = e.button.x;
			out.y = e.button.y;
			return true;
		}
		break;
	case SDL_MOUSEMOTION:
		out.type = GUIEvent::POINTER_MOVE;
		out.x = e.motion.x;
		out.y = e.motion.y;
		out.dx = e.motion.xrel;
		out.dy = e.motion.yrel;
		return true;
	default:
		break;
	}
	return false;
}

int GUISystem::getElementAtPosition(int x, int y) {
	for (int i=0; i<elements_.size(); i++) {
		if (elements_[i].get()
			&& elements_[i]->area_.containsPoint(x, y)
			&& elements_[i]->containsPoint(x, y)
		)
			return i;
	}
	return -1;
}

bool GUISystem::handleEvent(SDL_Event const& sdl_ev) {
	GUIEvent ev;
	if (!translateEvent(sdl_ev, ev))
		return false;	// not an useful event
	int targetElement = -1;
	std::vector<std::pair<int, GUIEvent>> additionalEvents; // first is target, second is event
	switch (ev.type) {
	case GUIEvent::TOUCH_BEGIN:
		assert(activeTouchElementHandle_ == -1);
		activeTouchElementHandle_ = getElementAtPosition(ev.x, ev.y);
		ev.dx = ev.dy = 0;
		touchMoveDelta_ = 0;
		if (elementBelowPointerHandle_ != activeTouchElementHandle_) {
			if (elementBelowPointerHandle_ >= 0) {
				GUIEvent additional = ev;
				additional.type = GUIEvent::POINTER_LEAVE;
				additionalEvents.push_back({elementBelowPointerHandle_, additional});
			}
			elementBelowPointerHandle_ = activeTouchElementHandle_;
			if (activeTouchElementHandle_ >= 0) {
				GUIEvent additional = ev;
				additional.type = GUIEvent::POINTER_ENTER;
				additionalEvents.push_back({activeTouchElementHandle_, additional});
			}
		}
		break;
	case GUIEvent::TOUCH_END: {
		targetElement = activeTouchElementHandle_;
		activeTouchElementHandle_ = -1;
		ev.dx = ev.dy = 0;
		auto elementBelow = getElementAtPosition(ev.x, ev.y);
		if (touchMoveDelta_ < 5 && elementBelow == targetElement && elementBelow >= 0) {
			GUIEvent additional;
			additional.type = GUIEvent::CLICK;
			additionalEvents.push_back({elementBelow, additional});
		}
	} break;
	case GUIEvent::POINTER_MOVE: {
		if (activeTouchElementHandle_ >= 0) {
			ev.type = GUIEvent::TOUCH_DRAG;
		}
		touchMoveDelta_ += abs(ev.dx) + abs(ev.dy);
		int currentEl = getElementAtPosition(ev.x, ev.y);
		if (currentEl != elementBelowPointerHandle_) {
			if (elementBelowPointerHandle_ >= 0) {
				GUIEvent additional = ev;
				additional.type = GUIEvent::POINTER_LEAVE;
				additionalEvents.push_back({elementBelowPointerHandle_, additional});
			}
			elementBelowPointerHandle_ = currentEl;
			if (elementBelowPointerHandle_ >= 0) {
				GUIEvent additional = ev;
				additional.type = GUIEvent::POINTER_ENTER;
				additionalEvents.push_back({elementBelowPointerHandle_, additional});
			}
		}
		targetElement = currentEl;
	} break;
	default:
		throw std::runtime_error("unhandled event type");
		break;
	}
	if (activeTouchElementHandle_ >= 0)
		targetElement = activeTouchElementHandle_;
	for (auto &e : additionalEvents) {
		assert(e.first >= 0 && e.first < elements_.size() && elements_[e.first].get());
		e.second.x -= elements_[e.first]->area_.x;
		e.second.y -= elements_[e.first]->area_.y;
		elements_[e.first]->handleEvent(e.second);
	}
	if (targetElement >= 0) {
		assert(targetElement < elements_.size() && elements_[targetElement].get());
		ev.x -= elements_[targetElement]->area_.x;
		ev.y -= elements_[targetElement]->area_.y;
		return elements_[targetElement]->handleEvent(ev);
	}
	return false;
}
