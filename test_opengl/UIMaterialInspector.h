#pragma once
#include "UIWindow.h"

class Material;
class Texture;
class Shader;

class UIMaterialInspector :
    public UIWindow
{
public:

    bool autoMerge;
    Material* currentMat;
    Shader* shader;

    UIMaterialInspector(std::string _name);
    ~UIMaterialInspector();

private:

    int selectedMaterial;

    void LoadTextureTo(std::string path, std::string fileName, void* data);
    void SetTexture(std::string name, Texture** tex);
    void DisplayFinalTexture();
    void InputName();
    void SelectMaterial();
    void SendData();
    void WindowBody() override;
    void ClearSelection();
};

