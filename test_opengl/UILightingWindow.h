#pragma once
#include "UIWindow.h"
#include "Vector3.h"

class Lighting;
class Light;

class UILightingWindow :
    public UIWindow
{

public:

	Lighting* lighting;
	Light* currentLight;

	UILightingWindow(std::string _name, Lighting* _lighting);
	~UILightingWindow();

protected:

	void SunDirection();
	void SunColor();
	void SkyColor();
	void AmbientColor();
	void FloorColor();
	void LightPicker();
	void InputName();
	void ChangeLightType();
	void CommonLightData();
	void PointLight();
	void SpotLight();
	void LightInspector();

	virtual void WindowBody() override;
};

