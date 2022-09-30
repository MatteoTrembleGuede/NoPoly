#pragma once
#include "ShaderPart.h"
#include <list>
#include <glm/vec3.hpp>
#include "CustomPrimitive.h"

#define DYNAMIC_DATA_NUM 32

enum class PrimitiveShape
{
	Sphere,
	Box,
	BoxFrame,
	Torus,
	CappedTorus,
	Capsule,
	Link,
	Cylinder,
	Cone,
	Plane,
	Custom,
	PS_Count
};

std::string ToString(PrimitiveShape& shape);

typedef struct SphereData
{
	float radius;
}SphereData;

typedef struct BoxData
{
	Vector3 halfExtend;
}BoxData;

typedef struct BoxFrameData
{
	Vector3 halfExtend;
	float edgeThickness;
}BoxFrameData;

typedef struct TorusData
{
	float mainRadius;
	float auxRadius;
}TorusData;

typedef struct CappedTorusData
{
	float mainRadius;
	float auxRadius;
	float angle;
}CappedTorusData;

typedef struct LinkData
{
	float length;
	float mainRadius;
	float auxRadius;
}LinkData;

typedef struct ConeData
{
	float angle;
	float height;
}ConeData;

typedef struct CapsuleData
{
	float height;
	float radius;
}CapsuleData;

typedef struct CylinderData
{
	float height;
	float radius;
}CylinderData;

typedef union ShapeData
{
	SphereData sphere;
	BoxData box;
	BoxFrameData boxFrame;
	TorusData torus;
	CappedTorusData cappedTorus;
	LinkData link;
	ConeData cone;
	CapsuleData capsule;
	CylinderData cylinder;

	ShapeData();
	~ShapeData();
}ShapeData;

class Shader;
class Material;

class ShaderLeaf :
    public ShaderPart
{
public:

	ShaderLeaf();
	ShaderLeaf(ShaderPart* _parent);
	~ShaderLeaf();

	bool useColorPalette;
	Vector3* color;
	Vector3* lightingProperties;
	Vector3* palette;
	PrimitiveShape shape;
	ShapeData* shapeData;
	Material* material;
	ShaderFunction* customPimitive;

	void ResetShape(ShaderFunction* func);
	void SetShape(PrimitiveShape newShape);
	void SetCustomShape(ShaderFunction* _customShape);
	void Save(std::string& outSave) override;
	void Load(std::stringstream& inSave) override;
	virtual ShaderPart* Copy() override;
	bool IsDynamic();
	void SetDynamic(bool dynamic);

	static void Init();
	static void SendData(Shader* shader);

protected:

	int dynDataID;

	std::string GetColorCode();
	std::string GetLightingPropertiesCode();
	virtual void DrawShape(std::string& outCode, GenerationPass pass = Color) override;

	static ShapeData dynData[DYNAMIC_DATA_NUM];
	static glm::vec3 dynColor[DYNAMIC_DATA_NUM];
	static glm::vec3 dynLightingProps[DYNAMIC_DATA_NUM];
	static glm::vec3 dynPalette[DYNAMIC_DATA_NUM * 4];
	static std::list<ShaderLeaf*> dynLeaves;
	static int dynLeavesCount;

	static bool RequestDynamic(ShaderLeaf* leaf);
	static void ReleaseDynamic(ShaderLeaf* leaf);
};

