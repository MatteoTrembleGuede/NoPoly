#pragma once
#include "UIWindow.h"

class UIPopUp : public UIWindow
{
public:

	UIPopUp();
	UIPopUp(std::string _name);
	~UIPopUp();
	virtual void Display() override;
protected:

	virtual void WindowBody() override;
};

