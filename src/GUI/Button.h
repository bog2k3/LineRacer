#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "GUIElement.h"

#include <string>
#include <functional>

class Button : public GUIElement {
public:
	Button(int x, int y, int w, int h);
	virtual ~Button() override {}

	virtual void draw(Viewport*) override;

	void setText(std::string text);
	void setIcon(...);
	void setAction(std::function<void(Button*)> callback);

protected:
	virtual bool onPointerEnter(int x, int y) override { hover_ = true; return true; }
	virtual bool onPointerLeave(int x, int y) override { hover_ = false; return true; }
	virtual bool onTouchBegin(int x, int y) override { pressed_ = true; return true; }
	virtual bool onTouchEnd(int x, int y) override;

	bool isHover() const { return hover_; }
	bool isPressed() const { return pressed_; }

private:
	std::string text_;
	std::function<void(Button*)> action_;
	bool hover_ = false;
	bool pressed_ = false;
};

#endif //__BUTTON_H__
