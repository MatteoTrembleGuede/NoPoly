#pragma once

#include "ShaderPart.h"
#include <list>

class ShaderNode : public ShaderPart
{
public:

	std::list<ShaderPart*> parts;

	ShaderNode();
	ShaderNode(ShaderPart* _parent);
	~ShaderNode() override;

	void Save(std::string& outSave);
	void Load(std::stringstream& inSave);
	virtual ShaderPart* Copy() override;
	virtual void GenerateBounds(std::string& outCode, std::list<std::string>& boolNames) override;

protected:

	void DrawShape(std::string& outCode, GenerationPass pass = Color) override;

};

