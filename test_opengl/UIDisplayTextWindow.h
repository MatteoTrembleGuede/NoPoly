#pragma once
#include "UIWindow.h"
#include "BalazsJakoTextEditor/TextEditor.h"

class UIDisplayTextWindow :
    public UIWindow
{
public:

    UIDisplayTextWindow(std::string _name, std::string _text);
    ~UIDisplayTextWindow();

private:

    TextEditor editor;

protected:

    void WindowBody() override;

};

