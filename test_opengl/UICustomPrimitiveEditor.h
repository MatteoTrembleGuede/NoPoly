#pragma once
#include "UIWindow.h"
#include "CustomPrimitive.h"
#include "BalazsJakoTextEditor/TextEditor.h"



class UIShaderFunctionEditor :
    public UIWindow
{
private:

    FunctionType currentType = FunctionType(0);
    int selectedModel = -1;
    ShaderFunctionModel* currentModel;
	TextEditor editor;
    std::string paramTypes[AttribType::AT_Count];
    std::string funcTypes[FunctionType::FT_Count];

	static std::string noneSelected;

    void LoadModel(std::string path, std::string fileName, void* data);

public:

    UIShaderFunctionEditor(std::string _name);

protected:

    void SaveLoadModel();
    void ChooseType();
    void PickModel();
    void AddModel();
    void SignModel();
    void EditFunctionBody();
    void WindowBody() override;
};

