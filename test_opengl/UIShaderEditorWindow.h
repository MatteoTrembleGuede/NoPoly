#pragma once
#include "UIWindow.h"
#include "ShaderGenerator.h"
#include "ShaderLeaf.h"
#include "BalazsJakoTextEditor/TextEditor.h"
#include "FPSCounter.h"

class UIShaderEditorWindow :
    public UIWindow
{
public:

    UIShaderEditorWindow(std::string _name, ShaderGenerator* _shader);
	~UIShaderEditorWindow();

    void Load(std::string path, std::string fileName, void* data);
	void CompileShader();
    void UpdateRenderFrameRate();

	static char* noMatText;

private:
    
    ShaderGenerator* shader;
    ShaderPart* currentPart;
    bool isLeaf;
    bool addingNewPart;
    bool showingGeneratedCode;
    bool compilationError;
    std::string errorMsg;
    std::string errorCode;
	TextEditor editor;
    FPSCounter mainFPSCounter;
    FPSCounter renderFPSCounter;
    int SRToSwap = -1;
    int swapDir = 0;
    int selMask;

    char* primitiveTypesNames[(int)PrimitiveShape::PS_Count];

    void ResetSelected();
    void SetPartTarget(ShaderPart* _part);
    void Save(std::string path, std::string fileName, void* data);


    // generic ui
    void InputName();
    void Menu();
    void CompileButton();
    void BlendModeButton();
    void ReturnButton();
    void DisplayError();
    void TransformPoint();
    void AnimateScale();
    void AnimatePosition();
    void ChooseSpaceRemap(ShaderFunction*& func);
    void RemapSpace();
    //void AnimateDistance();
    void Elongate();
    void Repeat();
    void RevolutionRepeat();
    void RotateObject();
    void RoundShape();
   
    // leaf functions
    void LeafBody();
    void BoxDataSetter();
    void BoxFrameDataSetter();
    void CappedTorusDataSetter();
    void CapsuleDataSetter();
    void ConeDataSetter();
    void CylinderDataSetter();
    void LinkDataSetter();
    void SphereDataSetter();
    void TorusDataSetter();
    void ShaderFunctionDataSetter(ShaderFunction* function);
    void CustomDataSetter();
    void ChoosePrimitiveShape();
    void ChooseCustomPrimitive();

    // node functions
    void Group();
    void UnGroup();
    void NodeBody();
    void NewPart();

protected:

    void ChooseMaterial();
    void WindowBody() override;

};

