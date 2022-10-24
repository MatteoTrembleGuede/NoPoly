#pragma once

class UIShaderEditorWindow;

class MenuBar
{
private:

	UIShaderEditorWindow* editorWindow;

public:

	MenuBar(UIShaderEditorWindow* _editorWindow) { editorWindow = _editorWindow; };

	void Display();
};

