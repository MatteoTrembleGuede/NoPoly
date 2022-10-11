#pragma once
#include "UIWindow.h"
#include <list>

class UIHost :
    public UIWindow
{
public:

    UIHost(std::string _name);
    ~UIHost();

    void AddChild(UIWindow* window);

protected:

    virtual void WindowBody() override;

private:

    UIWindow* draggedWindow = nullptr;
    std::list<UIWindow*> children;

};

