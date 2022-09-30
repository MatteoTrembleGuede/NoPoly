#pragma once

class Camera;
class Transformable;

class Guizmo
{
public:

	static void Init();
	static void Update(Camera* camera);
	static void SetTarget(Transformable* newTarget);
	static void SetVisible(bool visible);
	static bool IsVisible() { return isVisible; }
	static void SetOperation(int operation);
	static void SetWorld(bool world);
	static bool IsWorld();

private:

	static Transformable* target;
	static bool isVisible;
	static int manipulationOP;
	static int manipulationMode;

};

