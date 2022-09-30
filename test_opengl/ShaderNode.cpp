#include "ShaderNode.h"
#include <iostream>
#include <sstream>
#include "ShaderLeaf.h"

ShaderNode::ShaderNode()
{
}

ShaderNode::ShaderNode(ShaderPart* _parent) : ShaderPart(_parent)
{

}

ShaderNode::~ShaderNode()
{
	for (auto it = parts.begin(); it != parts.end(); ++it)
	{
		delete (*it);
	}
}

void ShaderNode::DrawShape(std::string& outCode, GenerationPass pass)
{
	outCode +=
		Tab(layer + 2) + "float minDist" + Layer + " = 999999999;\n"
		//+ Tab(layer + 2) + "float totalWeight" + Layer + " = 0.0f;\n"
		;

	for (auto it = parts.begin(); it != parts.end(); ++it)
	{
		(*it)->GenerateCode(outCode, layer + 1, pass);
	}

	outCode += Tab(layer + 2) + "tmpDist = minDist" + Layer + ";\n";
	//outCode += Tab(layer + 2) + "color" + Layer + " /= totalWeight" + Layer + ";\n";
}

void ShaderNode::Save(std::string& outSave)
{
	outSave += "new group\n";
	ShaderPart::Save(outSave);

	for (auto it = parts.begin(); it != parts.end(); ++it)
	{
		(*it)->Save(outSave);
	}

	outSave += "endGroup\n";
}

void ShaderNode::Load(std::stringstream& inSave)
{
	bool stop = false;
	char buf[512];

	ShaderPart::Load(inSave);
	
	while (!stop)
	{
		inSave.getline(buf, 512, '\n');
		std::string line = buf;
		size_t fo = line.find_first_of(' ');
		fo = (fo == std::string::npos) ? line.size() : fo;
		std::string word = std::string(line.c_str(), fo);
		std::string value = std::string(line.c_str() + fo + 1);

		if (word == "new")
		{
			if (value == "object")
			{
				ShaderLeaf* leaf = new ShaderLeaf(this);
				leaf->Load(inSave);
				parts.push_back(leaf);
			}
			else if (value == "group")
			{
				ShaderNode* node = new ShaderNode(this);
				node->Load(inSave);
				parts.push_back(node);
			}
		}
		else if (word == "endGroup")
		{
			stop = true;
		}
	}
}

ShaderPart* ShaderNode::Copy()
{
	ShaderNode* node = (ShaderNode*)ShaderPart::Copy();
	node->parts.clear(); 

	for (auto it = parts.begin(); it != parts.end(); ++it)
	{
		ShaderPart* newPart = (*it)->Copy();
		newPart->SetParent(node);
		node->parts.push_back(newPart);
	}

	return node;
}
