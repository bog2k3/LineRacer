#ifndef __BUTTON_ROUND_H__
#define __BUTTON_ROUND_H__

#include "Button.h"

class ButtonRound : public Button {
public:
	ButtonRound(int centerX, int centerY, int radius);
	~ButtonRound() override {}
};

#endif //__BUTTON_ROUND_H__
