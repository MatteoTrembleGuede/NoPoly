#pragma once
#include "UIWindow.h"
#include <list>

class UIHost :
    public UIWindow
{
    friend class ViewportManager;
public:

    UIHost(std::string _name);
    ~UIHost();

    void AddChild(UIWindow* window);
    void RemoveChild(UIWindow* window);

protected:

    virtual void WindowBody() override;

private:

    UIWindow* draggedWindow = nullptr;
    std::list<UIWindow*> children;

};

