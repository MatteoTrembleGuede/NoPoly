#pragma once

#include <string>
#include "Transform.h"
#include "Transformable.h"

#define STR(a) std::to_string(a)
#define Layer STR(layer)
#define SubLayer (layer > 0 ? STR(layer - 1) : "")
#define Hollow(d) std::string(hollow && round ? ("abs(" d ")") : d)
#define Round (round ? STR(-roundValue) : "")
//#define r transform->rotation
//#define p transform->position
#define AnimatedScale (animateScale ? " * " + animatedScaleFunction : "")
#define Scale(m) ((animateScale || transform->scale != 1.0f) ? "InvScale(" + std::string(m) + "," + transform->GetScaleStr() + AnimatedScale + ")" : std::string(m))
#define AnimatedPosition(m) (animatePosition ? ("InvTranslate(" + std::string(m) + "," + animatedPositionFunction + ")") : std::string(m))
#define RepeatByRevolution(m, p) (revolutionRepeat ? ("RepeatByRevolution(" + std::string(m) + ", " + std::string(p) + ", " + STR(revolutionTimesRepeat) + ")") : std::string(m))

typedef enum BlendMode
{
	Add,
	Subtract,
	Intersect,
	Exclusion,
	Pierce,
	Count
}BlendMode;

typedef enum GenerationPass
{
	DistanceOnly,
	Color,
	LightingProp
}GenerationPass;

class ShaderFunction;

class ShaderPart : public Transformable
{
public:

	ShaderPart();
	ShaderPart(ShaderPart* _parent);
	virtual ~ShaderPart();

	std::string name;
	int matID;
	
	Transform* transform;
	bool transformPoint = true;

	bool elongate = false;
	Vector3 elongation;

	bool round = false;
	bool hollow = false;
	float roundValue;
	
	bool repeat = false;
	Vector3 repeatOffset;
	
	bool revolutionRepeat = false;
	float revolutionTimesRepeat;
	
	bool rotating = false;
	float rotatingSpeed = 1.0f;
	
	bool animateScale = false;
	std::string animatedScaleFunction;
	
	bool animatePosition = false;
	std::string animatedPositionFunction;

	float bias = 0;
	
	BlendMode blendMode;
	float smooth;

	bool emittingLight = false;
	
	int layer;

	std::list<ShaderFunction*> spaceRemaps;

	void RemoveSpaceRemap(ShaderFunction* func);
	void AddSpaceRemap();
	void ReplaceSpaceRemap(ShaderFunction* oldFunc, ShaderFunction* newFunc);
	void GenerateCode(std::string& outCode, int _layer, GenerationPass pass = Color);
	ShaderPart* GetParent();
	void SetParent(ShaderPart* _parent) { parent = _parent; };
	virtual void Save(std::string& outSave);
	virtual void Load(std::stringstream& inSave);
	virtual ShaderPart* Copy();

	virtual glm::mat4 GetLocalBase() override;
	virtual glm::vec3 GetPosition() override;
	virtual glm::vec3 GetRotation() override;
	virtual glm::vec3 GetScale() override;
	virtual void SetPosition(glm::vec3 _position) override;
	virtual void SetRotation(glm::vec3 _rotation) override;
	virtual void SetScale(glm::vec3 _scale) override;
	virtual bool IsDynamic() override;

protected:

	ShaderPart* parent;

	void TransformPoint(std::string& outCode, GenerationPass pass);
	virtual void DrawShape(std::string& outCode, GenerationPass pass = Color) = 0;
	void BlendShape(std::string& outCode, GenerationPass pass = Color);
	std::string Tab(int _offset);
};

