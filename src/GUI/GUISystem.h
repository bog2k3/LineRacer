#ifndef __GUI_SYSTEM_H__
#define __GUI_SYSTEM_H__

#include "GUIElement.h"

#include <map>
#include <memory>
#include <vector>

union SDL_Event;

class GUISystem {
public:
	GUISystem() = default;
	~GUISystem() {}

	int addElement(std::unique_ptr<GUIElement> &&e); // returns the element handle that can be used to remove it or manipulate it
	void removeElement(int elementHandle);
	GUIElement* getElement(int elementHandle);

	void render(SDL_Renderer* r);
	bool handleEvent(SDL_Event const& e);	// returns true if event was consumed, false if ignored

private:
	std::vector<std::unique_ptr<GUIElement>> elements_;
	int activeTouchElementHandle_ = -1;
	int elementBelowPointerHandle_ = -1;
	int touchMoveDelta_ = 0;

	bool translateEvent(SDL_Event const& e, GUIEvent &out);
	int getElementAtPosition(int x, int y);
};

#endif //__GUI_SYSTEM_H__
