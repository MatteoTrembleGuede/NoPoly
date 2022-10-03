#pragma once

#include "UIPopUp.h"
#include "CustomPrimitive.h"


class UIAttribDesc :
    public UIPopUp
{
public:

    UIAttribDesc(std::string _name, AttribModel& _attrib) : UIPopUp(_name), attrib(_attrib) {};

private:

    AttribModel& attrib;

protected:

    virtual void WindowBody() override;
};

