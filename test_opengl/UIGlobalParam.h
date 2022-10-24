#pragma once
#include "UIWindow.h"
#include "Shader.h"

enum NegativeCorrectionMode
{
	None,
	Post,
	During,
	NCCount
};

class UIGlobalParam :
    public UIWindow
{

private:

	Shader* shader;

	static NegativeCorrectionMode correctionMode;
	static int NCStepNum;
	static int DRMNCPrecision;
	static float AORadius;
	static float AOStrength;
	static float normalRadius;
	static float shadowSharpness;

	void SendGlobalConfig();

public:

    UIGlobalParam(std::string _name, Shader* _shader);
    ~UIGlobalParam();

	virtual void WindowBody() override;

};

