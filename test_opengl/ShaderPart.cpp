#include "ShaderPart.h"
#include "ShaderNode.h"
#include "ShaderLeaf.h"
#include "CustomPrimitive.h"
#include <iostream>
#include <sstream>
#include "ShaderGenerator.h"


ShaderPart::ShaderPart()
{
	parent = nullptr;
	layer = 0;
	blendMode = BlendMode::Add;
	smooth = 0.0f;
	name = "new object";
	transform = new Transform();
	matID = -1;
}

ShaderPart::ShaderPart(ShaderPart* _parent)
{
	parent = _parent;
	layer = _parent->layer + 1;
	blendMode = BlendMode::Add;
	smooth = 0.0f;
	name = "new object";
	transform = new Transform();
	matID = -1;
}

ShaderPart::~ShaderPart()
{
	for (ShaderFunction* func : spaceRemaps)
	{
		delete func;
	}
	spaceRemaps.clear();
	delete transform;
}

std::string ShaderPart::Tab(int _offset)
{
	std::string out;
	for (int i = 0; i < _offset; ++i)
	{
		out += '\t';
	}
	return out;
}

glm::vec3 ShaderPart::GetPosition()
{
	return -glm::vec3(transform->position.x, transform->position.y, transform->position.z);
}

glm::vec3 ShaderPart::GetRotation()
{
	return glm::degrees(glm::vec3(transform->rotation.x, transform->rotation.y, transform->rotation.z));
}

glm::vec3 ShaderPart::GetScale()
{
	return glm::vec3(transform->scale, transform->scale, transform->scale);
}

void ShaderPart::SetPosition(glm::vec3 _position)
{
	transform->position = (*(Vector3*)&_position) * -1.0f;
}

void ShaderPart::SetRotation(glm::vec3 _rotation)
{
	_rotation = glm::radians(_rotation);
	transform->rotation = (*(Vector3*)&_rotation);
}

void ShaderPart::SetScale(glm::vec3 _scale)
{
	glm::vec3 scaleDiff = glm::vec3(fabs(_scale.x - transform->scale), fabs(_scale.y - transform->scale), fabs(_scale.z - transform->scale));
	transform->scale = scaleDiff.x > 0 ? _scale.x : scaleDiff.y > 0 ? _scale.y : _scale.z;
}

bool ShaderPart::IsDynamic()
{
	return transform->IsDynamic();
}

void ShaderPart::RemoveSpaceRemap(ShaderFunction* func)
{
	func->OnModelWillDestroy.Unbind(this, &ShaderPart::RemoveSpaceRemap);
	spaceRemaps.remove(func);
	func->model.onModelDeleted += [func] {
		delete func;
	};
}

void ShaderPart::AddSpaceRemap()
{
	if (ShaderFunction::models[FunctionType::SpaceRemap].size() > 0)
	{
		ShaderFunction* newFunc = new ShaderFunction(*(ShaderFunction::models[FunctionType::SpaceRemap].front()));
		newFunc->OnModelWillDestroy.Bind(this, &ShaderPart::RemoveSpaceRemap);
		spaceRemaps.push_back(newFunc);
	}
}

void ShaderPart::ReplaceSpaceRemap(ShaderFunction* oldFunc, ShaderFunction* newFunc)
{
	for (auto it = spaceRemaps.begin(); it != spaceRemaps.end(); ++it)
	{
		if (*it == oldFunc)
		{
			oldFunc->OnModelWillDestroy.Unbind(this, &ShaderPart::RemoveSpaceRemap);
			newFunc->OnModelWillDestroy.Bind(this, &ShaderPart::RemoveSpaceRemap);
			(*it) = newFunc;
			delete oldFunc;
			return;
		}
	}

	delete newFunc;
}

#define BOOL_NAME (std::string("obj_") + std::to_string((unsigned int)this) + "_IsVisible")
void ShaderPart::GenerateBounds(std::string& outCode, std::list<std::string>& boolNames)
{
	if (useBoundingVolume)
	{
		std::string boolName = BOOL_NAME;
		boolNames.push_back(boolName);
		glm::mat4 mat = glm::inverse(GetLocalBase() * transform->GetMatrix());
		glm::vec4 pos4 = (mat * glm::vec4(0, 0, 0, 1));
		glm::vec3 pos = glm::vec3(pos4.x, pos4.y, pos4.z) + boundingVolume.offset;

		outCode += "	if (";
		outCode += BOOL_NAME + " = ";

		switch (boundingVolume.type)
		{
		case BoundingVolume::Type::Box:
		{
			glm::vec3 minPos = pos - boundingVolume.size.b;
			glm::vec3 maxPos = pos + boundingVolume.size.b;

			outCode += std::string("RayBoxIntersection(vec3(") + std::to_string(minPos.x) + "," + std::to_string(minPos.y) + "," + std::to_string(minPos.z) +
				"), vec3(" + std::to_string(maxPos.x) + "," + std::to_string(maxPos.y) + "," + std::to_string(maxPos.z) +
				"), camPos, direction * 300, tmpDist)";
			break;
		}
		case BoundingVolume::Type::Sphere:
		{
			outCode += std::string("RaySphereIntersection(camPos, direction, vec3(") +
				std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) +
				"), " + std::to_string(boundingVolume.size.s) +
				", tmpDist)";
			break;
		}
		case BoundingVolume::Type::Ellipsoid:
		{
			outCode += std::string("RayEllipsoidIntersection(camPos, direction, vec3(") +
				std::to_string(boundingVolume.size.b.x) + "," + std::to_string(boundingVolume.size.b.y) + "," + std::to_string(boundingVolume.size.b.z) +
				"), vec3(" + std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) +
				"), tmpDist)";
			break;
		}
		}

		outCode += ")\n"
			"	{\n"
			"		smallestDist = min(smallestDist, tmpDist);\n"
			"		color = mix(color, vec3(" + std::to_string(boundingVolume.color.x) + "," + std::to_string(boundingVolume.color.y) + "," + std::to_string(boundingVolume.color.z) + "), 0.2);\n"
			"	}\n";
	}
}

void ShaderPart::GenerateCode(std::string& outCode, int _layer, GenerationPass pass)
{
	if (hide) return;
	layer = _layer;
	outCode += "\n" + Tab(layer + 1) + "// " + name + "\n" + Tab(layer + 1);

	if (useBoundingVolume) outCode += std::string("if (") + BOOL_NAME + ")";

	outCode += "{\n";

	if (pass == Color)
	{
		outCode += Tab(layer + 2) + "vec3 color" + Layer + ";\n";
	}
	else if (pass == LightingProp)
	{
		outCode += Tab(layer + 2) + "vec3 lp" + Layer + ";\n";
	}
	TransformPoint(outCode, pass);
	DrawShape(outCode, pass);
	BlendShape(outCode, pass);
	outCode += "\n" + Tab(layer + 1) + "}\n\n";
}

ShaderPart* ShaderPart::GetParent()
{
	return parent;
}

void ShaderPart::Save(std::string& outSave)
{
	outSave += "BoundingV \n";
	outSave += "Used " + STR(useBoundingVolume) + "\n";
	outSave += "Type " + STR(boundingVolume.type) + "\n";
	outSave += "Size " + STR(boundingVolume.size.b.x) + " " + STR(boundingVolume.size.b.y) + " " + STR(boundingVolume.size.b.z) + "\n";
	outSave += "Offset " + STR(boundingVolume.offset.x) + " " + STR(boundingVolume.offset.y) + " " + STR(boundingVolume.offset.z) + "\n";
	outSave += "Color " + STR(boundingVolume.color.x) + " " + STR(boundingVolume.color.y) + " " + STR(boundingVolume.color.z) + "\n";
	
	if (spaceRemaps.size() > 0)
	{
		outSave += "spaceRemaps \n";
		for (ShaderFunction*& func : spaceRemaps)
		{
			func->Save(outSave);
		}
	}

	if (bias != 0.0f) outSave += "bias " + STR(bias) + "\n";
	if (emittingLight) outSave += "emitLight \n";

	outSave += "name " + name + "\n";
	outSave += "transformPoint " + STR(transformPoint) + "\n";
	outSave += "dynTransform " + STR(transform->IsDynamic()) + "\n";
	outSave += "position " + STR(transform->position.x) + " " + STR(transform->position.y) + " " + STR(transform->position.z) + "\n";
	outSave += "rotation " + STR(transform->rotation.x) + " " + STR(transform->rotation.y) + " " + STR(transform->rotation.z) + "\n";
	outSave += "scale " + STR(transform->scale) + "\n";
	outSave += "animateScale " + STR(animateScale) + "\n";
	outSave += "animatedScaleFunction " + animatedScaleFunction + "\n";
	outSave += "animatePosition " + STR(animatePosition) + "\n";
	outSave += "animatedPositionFunction " + animatedPositionFunction + "\n";
	outSave += "elongate " + STR(elongate) + "\n";
	outSave += "elongation " + STR(elongation.x) + " " + STR(elongation.y) + " " + STR(elongation.z) + "\n";
	outSave += "round " + STR(round) + "\n";
	outSave += "hollow " + STR(hollow) + "\n";
	outSave += "roundValue " + STR(roundValue) + "\n";
	outSave += "repeat " + STR(repeat) + "\n";
	outSave += "repeatOffset " + STR(repeatOffset.x) + " " + STR(repeatOffset.y) + " " + STR(repeatOffset.z) + "\n";
	outSave += "revolutionRepeat " + STR(revolutionRepeat) + "\n";
	outSave += "revolutionTimesRepeat " + STR(revolutionTimesRepeat) + "\n";
	outSave += "rotating " + STR(rotating) + "\n";
	outSave += "rotatingSpeed " + STR(rotatingSpeed) + "\n";
	outSave += "smooth " + STR(smooth) + "\n";
	outSave += "blendMode " + STR((unsigned int)blendMode) + "\n";
	outSave += "matId " + STR(matID) + "\n";


}



void ShaderPart::Load(std::stringstream& inSave)
{
	char buf[512];
	inSave.getline(buf, 512, '\n');
	std::string line = buf;
	std::string word = std::string(line.c_str(), line.find_first_of(' '));
	std::string value = std::string(line.c_str() + line.find_first_of(' ') + 1);

	if (word == "BoundingV")
	{
		Vector3 tmp;
		for (int i = 0; i < 5; ++i)
		{
			inSave.getline(buf, 512, '\n');
			line = buf;
			word = std::string(line.c_str(), line.find_first_of(' '));
			value = std::string(line.c_str() + line.find_first_of(' ') + 1);

			if (word == "Used")
			{
				useBoundingVolume = std::stoi(value);
			}
			else if (word == "Type")
			{
				boundingVolume.type = (BoundingVolume::Type)std::stoi(value);
			}
			else if (word == "Size")
			{
				tmp = Vector3::Parse(value);
				boundingVolume.size.b = *(glm::vec3*)&tmp;
			}
			else if (word == "Offset")
			{
				tmp = Vector3::Parse(value);
				boundingVolume.offset = *(glm::vec3*)&tmp;
			}
			else if (word == "Color")
			{
				tmp = Vector3::Parse(value);
				boundingVolume.color = *(glm::vec3*)&tmp;
			}
		}

		inSave.getline(buf, 512, '\n');
		line = buf;
		word = std::string(line.c_str(), line.find_first_of(' '));
		value = std::string(line.c_str() + line.find_first_of(' ') + 1);
	}
	else
	{
		useBoundingVolume = false;
	}

	if (word == "spaceRemaps")
	{
		while (true)
		{
			inSave.getline(buf, 512, '\n');
			line = buf;
			word = std::string(line.c_str(), line.find_first_of(' '));
			value = std::string(line.c_str() + line.find_first_of(' ') + 1);

			if (word == "function")
			{
				ShaderFunctionModel* model = ShaderFunction::GetModel(value);
				ShaderFunction* func = new ShaderFunction(*model);
				func->Load(inSave);
				func->OnModelWillDestroy.Bind(this, &ShaderPart::RemoveSpaceRemap);
				spaceRemaps.push_back(func);
			}
			else if (word == "name" || word == "bias" || word == "emitLight")
			{
				break;
			}
		}
	}

	if (word == "bias")
	{
		bias = std::stof(value);

		inSave.getline(buf, 512, '\n');
		line = buf;
		word = std::string(line.c_str(), line.find_first_of(' '));
		value = std::string(line.c_str() + line.find_first_of(' ') + 1);
	}

	if (word == "emitLight")
	{
		emittingLight = true;

		inSave.getline(buf, 512, '\n');
		line = buf;
		word = std::string(line.c_str(), line.find_first_of(' '));
		value = std::string(line.c_str() + line.find_first_of(' ') + 1);
	}

	if (word == "name")
	{
		name = value;
	}

	for (int i = 0; i < 23; i++)
	{
		inSave.getline(buf, 512, '\n');
		line = buf;
		word = std::string(line.c_str(), line.find_first_of(' '));
		value = std::string(line.c_str() + line.find_first_of(' ') + 1);

		if (word == "position")
		{
			transform->position = Vector3::Parse(value);
		}
		else if (word == "rotation")
		{
			transform->rotation = Vector3::Parse(value);
		}
		else if (word == "smooth")
		{
			smooth = std::stof(value);
		}
		else if (word == "scale")
		{
			transform->scale = std::stof(value);
		}
		else if (word == "blendMode")
		{
			blendMode = (BlendMode)std::stoi(value);
		}
		else if (word == "transformPoint")
		{
			transformPoint = (bool)std::stoi(value);
		}
		else if (word == "dynTransform")
		{
			transform->SetDynamic((bool)std::stoi(value));
		}
		else if (word == "elongate")
		{
			elongate = (bool)std::stoi(value);
		}
		else if (word == "elongation")
		{
			elongation = Vector3::Parse(value);
		}
		else if (word == "round")
		{
			round = (bool)std::stoi(value);
		}
		else if (word == "roundValue")
		{
			roundValue = std::stof(value);
		}
		else if (word == "hollow")
		{
			hollow = (bool)std::stoi(value);
		}
		else if (word == "repeat")
		{
			repeat = (bool)std::stoi(value);
		}
		else if (word == "repeatOffset")
		{
			repeatOffset = Vector3::Parse(value);
		}
		else if (word == "revolutionRepeat")
		{
			revolutionRepeat = (bool)std::stoi(value);
		}
		else if (word == "revolutionTimesRepeat")
		{
			revolutionTimesRepeat = std::stof(value);
		}
		else if (word == "rotating")
		{
			rotating = (bool)std::stoi(value);
		}
		else if (word == "rotatingSpeed")
		{
			rotatingSpeed = std::stof(value);
		}
		else if (word == "animateScale")
		{
			animateScale = (bool)std::stoi(value);
		}
		else if (word == "animatedScaleFunction")
		{
			animatedScaleFunction = value;
		}
		else if (word == "animatePosition")
		{
			animatePosition = (bool)std::stoi(value);
		}
		else if (word == "animatedPositionFunction")
		{
			animatedPositionFunction = value;
		}
		else if (word == "matId")
		{
			matID = std::stoi(value);
		}
	}
}

ShaderPart* ShaderPart::Copy()
{
	ShaderPart* newPart;
	ShaderLeaf* leaf = dynamic_cast<ShaderLeaf*>(this);
	ShaderNode* node = dynamic_cast<ShaderNode*>(this);
	if (leaf)
	{
		newPart = (ShaderPart*)new ShaderLeaf(*leaf);
	}
	else
	{
		newPart = (ShaderPart*)new ShaderNode(*node);
	}

	newPart->transform = new Transform();
	newPart->transform->position = transform->position;
	newPart->transform->rotation = transform->rotation;
	newPart->transform->scale = transform->scale;
	newPart->transform->SetDynamic(transform->IsDynamic());

	for (ShaderFunction*& func : newPart->spaceRemaps)
	{
		func = new ShaderFunction(*func);
	}

	return newPart;
}

glm::mat4 ShaderPart::GetLocalBase()
{
	if (parent)
	{
		return parent->GetLocalBase() * parent->transform->GetMatrix();
	}
	else
	{
		return glm::mat4(1.0f);
	}
}

void ShaderPart::TransformPoint(std::string& outCode, GenerationPass pass)
{
	// define transform matrix

	outCode += Tab(layer + 2) + "mat4 m" + Layer + " = mat4(1, 0, 0, 0,\n";
	outCode += Tab(layer + 2) + "0, 1, 0, 0,\n";
	outCode += Tab(layer + 2) + "0, 0, 1, 0,\n";
	outCode += Tab(layer + 2) + "0, 0, 0, 1);\n";

	if (transformPoint)
	{
		outCode += Tab(layer + 2) + "m" + Layer + " = " + AnimatedPosition(Scale(transform->GetInvMatrixStr())) + "; \n";
	}

	outCode += Tab(layer + 2) + "m" + Layer + " = m" + Layer + " * " + RepeatByRevolution(("m" + SubLayer), (" tmpP" + SubLayer)) + ";\n";
	outCode += Tab(layer + 2) + "vec3 tmpP" + Layer + " = (m" + Layer + " * vec4(p, 1.0f)).xyz; \n";

	for (ShaderFunction* func : spaceRemaps)
	{
		if (!func) continue;
		outCode += Tab(layer + 2) + "m" + Layer + " = " + func->GenerateFunctionCall(layer) + ";\n";
		outCode += Tab(layer + 2) + "tmpP" + Layer + " = (m" + Layer + " * vec4(p, 1.0f)).xyz;\n";
	}

	if (repeat)
	{
		outCode += Tab(layer + 2) + "m" + Layer + " = InvTranslate(m" + Layer + ", (tmpP" + Layer + " - (mod(0.5*vec3(" + STR(repeatOffset.x) + "," + STR(repeatOffset.y) + "," + STR(repeatOffset.z) + ") + tmpP" + Layer + ",vec3(" + STR(repeatOffset.x) + "," + STR(repeatOffset.y) + "," + STR(repeatOffset.z) + "))-0.5*vec3(" + STR(repeatOffset.x) + "," + STR(repeatOffset.y) + "," + STR(repeatOffset.z) + "))));\n";
		outCode += Tab(layer + 2) + "tmpP" + Layer + " = (m" + Layer + " * vec4(p, 1.0f)).xyz;\n";
	}

	if (elongate)
	{
		outCode += Tab(layer + 2) + "m" + Layer + " = InvTranslate(m" + Layer + ", clamp( tmpP" + Layer + ", -vec3(" + STR(elongation.x) + "," + STR(elongation.y) + "," + STR(elongation.z) + "), vec3(" + STR(elongation.x) + "," + STR(elongation.y) + "," + STR(elongation.z) + ") ));\n";
		outCode += Tab(layer + 2) + "tmpP" + Layer + "= (m" + Layer + " * vec4(p, 1.0f)).xyz;\n";
	}

	if (rotating)
	{
		outCode += Tab(layer + 2) + "m" + Layer + " = Rotate(m" + Layer + ", time * 6.283185f * " + STR(rotatingSpeed) + ");\n";
		outCode += Tab(layer + 2) + "tmpP" + Layer + " = (m" + Layer + " * vec4(p, 1.0f)).xyz;\n";
	}
}

void ShaderPart::BlendShape(std::string& outCode, GenerationPass pass)
{
	if (transformPoint && (animateScale || transform->scale != 1.0f)) outCode += Tab(layer + 2) + "tmpDist *= " + transform->GetScaleStr() + AnimatedScale + ";\n";

	if (bias != 0.0f) outCode += Tab(layer + 2) + "tmpDist = -" + STR(bias) + " * tmpDist + tmpDist;\n";

	if (pass == GenerationPass::Color && matID >= 0)
	{
		outCode += "vec3 tmpColor" + SubLayer + " = color" + SubLayer + "; \n";
		outCode += "color" + SubLayer + " = vec3(0, 0, 0); \n";
		outCode += "color" + Layer + " = vec3(1, 0, 0); \n";
	}

	outCode += Tab(layer + 2) + "float smoothMin = ";
	if (pass == GenerationPass::DistanceOnly)
	{
		if (smooth != 0.0f)
		{
			switch (blendMode)
			{
			case BlendMode::Add:
				outCode += "Addition( minDist" + SubLayer + ", " + Hollow("tmpDist") + Round + ", " + STR(smooth) + "); \n";
				break;
			case BlendMode::Subtract:
				outCode += "Subtraction(" + Hollow("tmpDist") + Round + ", minDist" + SubLayer + ", " + STR(smooth) + "); \n";
				break;
			case BlendMode::Intersect:
				outCode += "Intersection(" + Hollow("tmpDist") + Round + ", minDist" + SubLayer + ", " + STR(smooth) + "); \n";
				break;
			case BlendMode::Exclusion:
				outCode += "Exclusion(" + Hollow("tmpDist") + Round + ", minDist" + SubLayer + ", " + STR(smooth) + "); \n";
				break;
			case BlendMode::Pierce:
				outCode += "AddAndSub( minDist" + SubLayer + ", " + Hollow("tmpDist") + Round + ", " + STR(smooth) + "); \n";
				break;
			}
		}
		else
		{
			switch (blendMode)
			{
			case BlendMode::Pierce:
			case BlendMode::Exclusion:
			case BlendMode::Add:
				outCode += "min( minDist" + SubLayer + ", " + Hollow("tmpDist") + Round + "); \n";
				break;
			case BlendMode::Subtract:
				outCode += "max(-(" + Hollow("tmpDist") + Round + "), minDist" + SubLayer + "); \n";
				break;
			case BlendMode::Intersect:
				outCode += "max((" + Hollow("tmpDist") + Round + "), minDist" + SubLayer + "); \n";
				break;
			}
		}
	}
	else if (pass == GenerationPass::Color)
	{

		if (smooth != 0.0f)
		{
			switch (blendMode)
			{
			case BlendMode::Add:
				outCode += "Addition(vec4(color" + SubLayer + ", minDist" + SubLayer + "), vec4(color" + Layer + ", " + Hollow("tmpDist") + Round + "), " + STR(smooth) + ", color" + SubLayer + "); \n";
				break;
			case BlendMode::Subtract:
				outCode += "Subtraction(vec4(color" + Layer + ", " + Hollow("tmpDist") + Round + "), vec4(color" + SubLayer + ", minDist" + SubLayer + "), " + STR(smooth) + ", color" + SubLayer + "); \n";
				break;
			case BlendMode::Intersect:
				outCode += "Intersection(vec4(color" + Layer + ", " + Hollow("tmpDist") + Round + "), vec4(color" + SubLayer + ", minDist" + SubLayer + "), " + STR(smooth) + ", color" + SubLayer + "); \n";
				break;
			case BlendMode::Exclusion:
				outCode += "Exclusion(vec4(color" + Layer + ", " + Hollow("tmpDist") + Round + "), vec4(color" + SubLayer + ", minDist" + SubLayer + "), " + STR(smooth) + ", color" + SubLayer + "); \n";
				break;
			case BlendMode::Pierce:
				outCode += "AddAndSub(vec4(color" + SubLayer + ", minDist" + SubLayer + "), vec4(color" + Layer + ", " + Hollow("tmpDist") + Round + "), " + STR(smooth) + ", color" + SubLayer + "); \n";
				break;
			}
		}
		else
		{
			switch (blendMode)
			{
			case BlendMode::Pierce:
			case BlendMode::Exclusion:
			case BlendMode::Add:
				outCode += "Min(vec4(color" + SubLayer + ", minDist" + SubLayer + "), vec4(color" + Layer + "," + Hollow("tmpDist") + Round + "), color" + SubLayer + "); \n";
				break;
			case BlendMode::Subtract:
				outCode += "Max(vec4(color" + Layer + ", -(" + Hollow("tmpDist") + Round + ")), vec4(color" + SubLayer + ", minDist" + SubLayer + "), color" + SubLayer + "); \n";
				break;
			case BlendMode::Intersect:
				outCode += "Max(vec4(color" + Layer + ", (" + Hollow("tmpDist") + Round + ")), vec4(color" + SubLayer + ", minDist" + SubLayer + "), color" + SubLayer + "); \n";
				break;
			}
		}

		if (matID >= 0)
		{
			outCode += Tab(layer + 2) + "UseMaterial(" + STR(matID) + ", color" + SubLayer + ".x, m" + Layer + "); \n";
			outCode += Tab(layer + 2) + "color" + SubLayer + " = tmpColor" + SubLayer + "; \n";
		}

		if (emittingLight)
		{
			outCode += Tab(layer + 2) + "vec3 lDir = mix(-n, normalize((InvMat(m" + Layer + ") * vec4(vec3(0), 1.0f)).xyz - p), clamp(-1.0/(smoothMin*500.0f+1)+1, 0, 1));\n";
			outCode += Tab(layer + 2) + "AddLight((0.9*smoothMin) * lDir + p, abs(0.9*smoothMin - 0.003), color" + Layer + "); \n";
		}
	}
	else if (pass == GenerationPass::LightingProp)
	{

		if (smooth != 0.0f)
		{
			switch (blendMode)
			{
			case BlendMode::Add:
				outCode += "Addition(vec4(lp" + SubLayer + ", minDist" + SubLayer + "), vec4(lp" + Layer + ", " + Hollow("tmpDist") + Round + "), " + STR(smooth) + ", lp" + SubLayer + "); \n";
				break;
			case BlendMode::Subtract:
				outCode += "Subtraction(vec4(lp" + Layer + ", " + Hollow("tmpDist") + Round + "), vec4(lp" + SubLayer + ", minDist" + SubLayer + "), " + STR(smooth) + ", lp" + SubLayer + "); \n";
				break;
			case BlendMode::Intersect:
				outCode += "Intersection(vec4(lp" + Layer + ", " + Hollow("tmpDist") + Round + "), vec4(lp" + SubLayer + ", minDist" + SubLayer + "), " + STR(smooth) + ", lp" + SubLayer + "); \n";
				break;
			case BlendMode::Exclusion:
				outCode += "Exclusion(vec4(lp" + Layer + ", " + Hollow("tmpDist") + Round + "), vec4(lp" + SubLayer + ", minDist" + SubLayer + "), " + STR(smooth) + ", lp" + SubLayer + "); \n";
				break;
			case BlendMode::Pierce:
				outCode += "AddAndSub(vec4(lp" + SubLayer + ", minDist" + SubLayer + "), vec4(lp" + Layer + ", " + Hollow("tmpDist") + Round + "), " + STR(smooth) + ", lp" + SubLayer + "); \n";
				break;
			}
		}
		else
		{
			switch (blendMode)
			{
			case BlendMode::Pierce:
			case BlendMode::Exclusion:
			case BlendMode::Add:
				outCode += "Min(vec4(lp" + SubLayer + ", minDist" + SubLayer + "), vec4(lp" + Layer + "," + Hollow("tmpDist") + Round + "), lp" + SubLayer + "); \n";
				break;
			case BlendMode::Subtract:
				outCode += "Max(vec4(lp" + Layer + ", -(" + Hollow("tmpDist") + Round + ")), vec4(lp" + SubLayer + ", minDist" + SubLayer + "), lp" + SubLayer + "); \n";
				break;
			case BlendMode::Intersect:
				outCode += "Max(vec4(lp" + Layer + ", (" + Hollow("tmpDist") + Round + ")), vec4(lp" + SubLayer + ", minDist" + SubLayer + "), lp" + SubLayer + "); \n";
				break;
			}
		}
	}
	outCode += Tab(layer + 2) + "minDist" + SubLayer + " = smoothMin; \n";
	//outCode += Tab(layer + 2) + "if (minDist" + SubLayer + " < RAYCAST_PRECISION * 10.0f) return minDist" + SubLayer + ";\n";
}
