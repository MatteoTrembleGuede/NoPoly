#pragma once
#include "UIWindow.h"
#include "Shader.h"
#include "BalazsJakoTextEditor/TextEditor.h"

class UIShaderTextEditorWindow :
    public UIWindow
{
public:
	UIShaderTextEditorWindow() = delete;
	UIShaderTextEditorWindow(Shader* _shader);
	~UIShaderTextEditorWindow();

protected:
	std::string text;
	Shader* shader;
	TextEditor editor;
	std::string errorMsg;
	bool compilationError;

	virtual void WindowBody() override;
};

