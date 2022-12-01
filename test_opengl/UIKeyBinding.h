#pragma once
#include "UIPopUp.h"
#include "Input/Input.h"

class UIKeyBinding :
    public UIWindow
{
public:

	UIKeyBinding(std::string _name);
	~UIKeyBinding();

protected:

	Input::DeviceID dID;
	Input::BindSet backup;

	virtual void WindowBody() override;
	void DisplayActions(Input::BindSet& bs);
	void DisplayAxes(Input::BindSet& bs);
};

