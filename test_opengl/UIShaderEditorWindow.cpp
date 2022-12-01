#include "UIShaderEditorWindow.h"
#include "ShaderLeaf.h"
#include "ShaderNode.h"
#include "GLFW/glfw3.h"
#include "Transform.h"
#include "Guizmo.h"
#include "ViewportManager.h"
#include "Material.h"
#include "UIResourceBrowser.h"
#include "UIDisplayTextWindow.h"
#include <bitset>
#include "Input/Input.h"

char* UIShaderEditorWindow::noMatText;

typedef bool (*DragFloatPtr) (const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags);

UIShaderEditorWindow::UIShaderEditorWindow(std::string _name, ShaderGenerator* _shader) : UIWindow(_name)
{
	auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	editor.SetLanguageDefinition(lang);
	editor.SetReadOnly(true);
	shader = _shader;
	SetPartTarget(static_cast<ShaderPart*>(shader->root));

	noMatText = new char[5];
	strcpy(noMatText, "None");

	for (int i = 0; i < (int)PrimitiveShape::PS_Count; i++)
	{
		PrimitiveShape shape = (PrimitiveShape)i;
		std::string name(ToString(shape));
		primitiveTypesNames[i] = new char[name.size() + 1];
		strcpy(primitiveTypesNames[i], name.c_str());
	}

	Input::GetGlobalInput(0).AddAction("CompileShader", Input::Key(Input::KeyCode::KP_ENTER));
	Input::GetGlobalInput(0).BindAction("CompileShader", Input::Mode::Press, this, &UIShaderEditorWindow::CompileShader);
}

UIShaderEditorWindow::~UIShaderEditorWindow()
{
	for (int i = 0; i < (int)PrimitiveShape::PS_Count; i++)
	{
		delete primitiveTypesNames[i];
	}
}

void UIShaderEditorWindow::BoxDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat3("half extends", (float*)&leaf->shapeData->box.halfExtend, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::BoxFrameDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat3("half extends", (float*)&leaf->shapeData->boxFrame.halfExtend, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("edge thickness", (float*)&leaf->shapeData->boxFrame.edgeThickness, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::CappedTorusDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat("main radius", (float*)&leaf->shapeData->cappedTorus.mainRadius, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("auxiliary radius", (float*)&leaf->shapeData->cappedTorus.auxRadius, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("aperture", (float*)&leaf->shapeData->cappedTorus.angle, 0.1f, 0, 3.141f);
}

void UIShaderEditorWindow::CapsuleDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat("height", (float*)&leaf->shapeData->capsule.height, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("radius", (float*)&leaf->shapeData->capsule.radius, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::ConeDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat("angle", (float*)&leaf->shapeData->cone.angle, 0.1f, -FLT_MAX, FLT_MAX);
	ImGui::DragFloat("height", (float*)&leaf->shapeData->cone.height, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::CylinderDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat("height", (float*)&leaf->shapeData->cylinder.height, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("radius", (float*)&leaf->shapeData->cylinder.radius, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::LinkDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat("length", (float*)&leaf->shapeData->link.length, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("main radius", (float*)&leaf->shapeData->link.mainRadius, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("auxiliary radius", (float*)&leaf->shapeData->link.auxRadius, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::SphereDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat("radius", (float*)&leaf->shapeData->sphere.radius, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::TorusDataSetter()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	ImGui::DragFloat("main radius", (float*)&leaf->shapeData->torus.mainRadius, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("auxiliary radius", (float*)&leaf->shapeData->torus.auxRadius, 0.1f, 0, FLT_MAX);
}

void UIShaderEditorWindow::ShaderFunctionDataSetter(ShaderFunction* function)
{
	for (auto it = function->attribs.begin(); it != function->attribs.end(); ++it)
	{
		Attrib* attrib = &(*it);
		DragFloatPtr func = nullptr;

		switch (attrib->model.type)
		{
		case AttribType::Float:
			func = ImGui::DragFloat;
			break;
		case AttribType::Vec2:
			func = ImGui::DragFloat2;
			break;
		case AttribType::Vec3:
			func = ImGui::DragFloat3;
			break;
		case AttribType::Vec4:
			func = ImGui::DragFloat4;
			break;
		}

		if (func)
		{
			float minimum = attrib->model.hasMin ? attrib->model.minimum : -FLT_MAX;
			float maximum = attrib->model.hasMax ? attrib->model.maximum : FLT_MAX;
			float step = attrib->model.hasStep ? attrib->model.step : 0.1;
			func(attrib->model.name.c_str(), attrib->val, step, minimum, maximum, "%.3f", 0);
		}
	}
}

void UIShaderEditorWindow::CustomDataSetter()
{
	ShaderLeaf* leaf = dynamic_cast<ShaderLeaf*>(currentPart);
	if (leaf && leaf->customPimitive)
	{
		ShaderFunctionDataSetter(leaf->customPimitive);
	}
}

void UIShaderEditorWindow::ChoosePrimitiveShape()
{
	ShaderLeaf* leaf = (ShaderLeaf*)currentPart;
	int selectedType = (int)leaf->shape;

	ImGui::Combo("Primitive Shape", &selectedType, primitiveTypesNames, (int)PrimitiveShape::PS_Count);

	if (selectedType != (int)leaf->shape)
	{
		leaf->SetShape((PrimitiveShape)selectedType);
		CompileShader();
	}
	ChooseCustomPrimitive();

	bool isDynamic = leaf->IsDynamic();

	if (ImGui::Checkbox("dynamic data", &isDynamic))
	{
		leaf->SetDynamic(isDynamic);
		CompileShader();
	}

	if (!leaf->IsDynamic())
	{
		ImGui::Text("static data need to compile to see modifications");
	}
}

bool PickModel(FunctionType type, int& selected)
{
	return ImGui::Combo("Select custom function", &selected,
		[](void* data, int idx, const char** out_text)
		{
			std::list<ShaderFunctionModel*>* modelList = (std::list<ShaderFunctionModel*>*)data;
			if (idx >= 0 && idx < modelList->size())
			{
				auto it = modelList->begin();
				for (int i = 0; i < idx; ++i) ++it;
				*out_text = (*it)->name.c_str();
				return true;
			}
			else
			{
				return false;
			}
		}
	, &ShaderFunction::models[type], ShaderFunction::models[type].size());
}

void UIShaderEditorWindow::ChooseCustomPrimitive()
{
	ShaderLeaf* leaf = dynamic_cast<ShaderLeaf*>(currentPart);
	if (!leaf || leaf->shape != PrimitiveShape::Custom) return;
	int selected = leaf->customPimitive ? leaf->customPimitive->model.ID : -1;

	if (PickModel(FunctionType::Primitive, selected))
	{
		auto it = ShaderFunction::models[FunctionType::Primitive].begin();
		for (int i = 0; i < selected; ++i) ++it;
		leaf->SetCustomShape(new ShaderFunction(*(*it)));
	}
}

void UIShaderEditorWindow::LeafBody()
{
	ShaderLeaf* part = static_cast<ShaderLeaf*>(currentPart);
	ImGui::Text("\n\nPrimitive type : %s\n\n", ToString(part->shape).c_str());
	ChoosePrimitiveShape();

	ImGui::Text("\nSwitching color/palette need compilation to apply");
	ImGui::Checkbox("use color palette", &part->useColorPalette);
	if (part->useColorPalette)
	{
		ImGui::DragFloat3("bias", (float*)part->palette, 0.05f, 0.0f, 100.0f);
		ImGui::DragFloat3("magnitude", (float*)(part->palette + 1), 0.05f, 0.0f, 100.0f);
		ImGui::DragFloat3("frequency", (float*)(part->palette + 2), 0.05f, 0.0f, 100.0f);
		ImGui::DragFloat3("phase", (float*)(part->palette + 3), 0.05f, 0.0f, 100.0f);
	}
	else
	{
		ImGui::ColorEdit3("color", (float*)part->color);
	}

	ImGui::DragFloat("Metallic", (float*)part->lightingProperties, 0.01f, 0, 1);
	ImGui::DragFloat2("Shine/Gloss", (float*)part->lightingProperties + 1, 0.1f, 0);

	switch (part->shape)
	{
	case PrimitiveShape::Box:
		BoxDataSetter();
		break;
	case PrimitiveShape::BoxFrame:
		BoxFrameDataSetter();
		break;
	case PrimitiveShape::CappedTorus:
		CappedTorusDataSetter();
		break;
	case PrimitiveShape::Capsule:
		CapsuleDataSetter();
		break;
	case PrimitiveShape::Cone:
		ConeDataSetter();
		break;
	case PrimitiveShape::Cylinder:
		CylinderDataSetter();
		break;
	case PrimitiveShape::Link:
		LinkDataSetter();
		break;
	case PrimitiveShape::Plane:
		break;
	case PrimitiveShape::Sphere:
		SphereDataSetter();
		break;
	case PrimitiveShape::Torus:
		TorusDataSetter();
		break;
	case PrimitiveShape::Custom:
		CustomDataSetter();
		break;
	default:
		break;
	}
}

void UIShaderEditorWindow::NewPart()
{
	ShaderNode* node = dynamic_cast<ShaderNode*>(currentPart);
	if (!node) return;

	if (node->GetParent())
	{
		if (ImGui::Button("Ungroup this part"))
		{
			UnGroup();
		}
	}

	if (selMask)
	{
		if (ImGui::Button("group selected parts"))
		{
			Group();
		}
		ImGui::SameLine();
		if (ImGui::Button("hide"))
		{
			Hide(true);
		}
		ImGui::SameLine();
		if (ImGui::Button("unhide"))
		{
			Hide(false);
		}
	}
	else
	{
		if (ImGui::Button("group this part"))
		{
			if (currentPart->GetParent())
			{
				for (auto it = ((ShaderNode*)currentPart->GetParent())->parts.begin();
					it != ((ShaderNode*)currentPart->GetParent())->parts.end(); ++it)
				{
					if (*it == currentPart)
					{
						shader->InsertGroupBefore(*it);
						break;
					}
				}
			}
			else
			{
				shader->InsertGroupBefore(shader->root);
			}
			CompileShader();
			currentPart = currentPart->GetParent();
		}
	}

	ImGui::Text("\n");

	if (ImGui::Button("add new part") || addingNewPart)
	{
		addingNewPart = true;
		ImGui::OpenPopup("new part type ?");
		ImGui::BeginPopup("new part type ?");
		if (ImGui::Button("Node/group"))
		{
			addingNewPart = false;
			node->parts.push_back(new ShaderNode(node));
			CompileShader();
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("leaf/object"))
		{
			addingNewPart = false;
			node->parts.push_back(new ShaderLeaf(node));
			CompileShader();
			ImGui::CloseCurrentPopup();
		}

		ImVec2 popUpSize, popUpPos, mousePos;
		popUpSize = ImGui::GetWindowSize();
		popUpPos = ImGui::GetWindowPos();
		mousePos = ImGui::GetMousePos();
		if (ImGui::IsAnyMouseDown() && !ImGui::IsMouseHoveringRect(popUpPos, ImVec2(popUpPos.x + popUpSize.x, popUpPos.y + popUpSize.y)))
		{
			addingNewPart = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void UIShaderEditorWindow::Hide(bool hide)
{
	ShaderNode* node = (ShaderNode*)currentPart;
	int i = 0;
	for (auto it = node->parts.begin(); it != node->parts.end(); ++it)
	{
		if (selMask >> i & 1)
		{
			(*it)->hide = hide;
		}
		++i;
	}
	selMask = 0;
	CompileShader();
}

void UIShaderEditorWindow::Group()
{
	ShaderNode* node = (ShaderNode*)currentPart;
	ShaderNode* newNode = new ShaderNode(node);
	newNode->name = "new group";
	node->parts.push_back(newNode);
	int i = 0;
	for (auto it = node->parts.begin(); it != node->parts.end(); ++it)
	{
		if (selMask >> i & 1)
		{
			newNode->parts.push_back(*it);
			(*it)->SetParent(newNode);
		}
		++i;
	}
	for (auto it = newNode->parts.begin(); it != newNode->parts.end(); ++it)
	{
		node->parts.remove(*it);
	}
	selMask = 0;
	CompileShader();
}

void UIShaderEditorWindow::UnGroup()
{
	ShaderNode* node = (ShaderNode*)currentPart;
	ShaderNode* parent = (ShaderNode*)node->GetParent();
	for (auto it = node->parts.begin(); it != node->parts.end(); ++it)
	{
		ShaderPart* part = *it;
		part->SetParent(parent);
		parent->parts.push_back(part);
	}

	parent->parts.remove(node);
	node->parts.clear();
	delete node;
	SetPartTarget(parent);
	CompileShader();
	ResetSelected();
}

void UIShaderEditorWindow::NodeBody()
{
	ShaderNode* node = (ShaderNode*)currentPart;

	ImGui::Text("\n\nchildren list : ");
	NewPart();

	if (node != currentPart) return;

	int id = 0;
	int width = ImGui::GetWindowSize().x;
	for (auto it = node->parts.begin(); it != node->parts.end(); ++it)
	{
		ImGui::BeginChild(id + 1, ImVec2(width, 25));
		bool mustBreak = false;
		if (ImGui::BeginTable("children list : ", 3))
		{

			ShaderNode* childNode = dynamic_cast<ShaderNode*>(*it);
			ShaderLeaf* childLeaf = dynamic_cast<ShaderLeaf*>(*it);
			auto itNext = it;
			++itNext;
			auto itPrev = it;
			if (it != node->parts.begin())
			{
				--itPrev;
			}

			ImGui::TableNextColumn();

			// selection

			int m = selMask;
			bool check = (m >> id) & 1;

			ImGui::Checkbox("", &check);
			m = 1 << id;
			if (check)
				selMask |= m;
			else
				selMask &= ~m;

			//

			ImGui::SameLine();

			if (childNode)
			{
				if (ImGui::Button(("grp " + STR(id) + " : " + (childNode->hide ? "(hidden)" : "") + childNode->name).c_str()))
				{
					SetPartTarget(childNode);
				}
			}
			else if (childLeaf)
			{
				if (ImGui::Button(("obj " + STR(id) + " : " + (childLeaf->hide ? "(hidden)" : "") + childLeaf->name).c_str()))
				{
					SetPartTarget(childLeaf);
				}
			}
			else
			{
				ImGui::Text(("invalid child " + STR(id)).c_str());
			}

			ImGui::TableNextColumn();
			if (ImGui::Button("up") && it != node->parts.begin())
			{
				std::swap(*itPrev, *it);
				CompileShader();
				ResetSelected();
			}

			ImGui::SameLine();

			if (ImGui::Button("down") && itNext != node->parts.end())
			{
				std::swap(*it, *itNext);
				CompileShader();
				ResetSelected();
			}
			ImGui::TableNextColumn();
			if (ImGui::Button("copy"))
			{
				ShaderPart* part = (*it)->Copy();
				part->SetParent(node);
				node->parts.push_back(part);
				CompileShader();
				ResetSelected();
				mustBreak = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("del"))
			{
				ShaderPart* part = (*it);
				node->parts.remove(part);
				delete part;
				CompileShader();
				ResetSelected();
				mustBreak = true;
			}
			++id;
			ImGui::EndTable();
		}
		ImGui::EndChild();
		if (mustBreak) break;
	}
}

void UIShaderEditorWindow::CompileShader()
{
	compilationError = !shader->Recompile(errorMsg, errorCode);
	editor.SetText(errorCode);
}

void UIShaderEditorWindow::CompileButton()
{
	if (ImGui::Button("Compile shader"))
	{
		CompileShader();
	}
}

void UIShaderEditorWindow::BlendModeButton()
{
	int selectedMode = (int)currentPart->blendMode;
	const char* modes[5]{ "Add" , "Subtract" , "Intersect" , "Exclude" , "Pierce" };

	ImGui::Combo("BlendMode", &selectedMode, modes, 5);

	if (selectedMode != (int)currentPart->blendMode)
	{
		currentPart->blendMode = (BlendMode)selectedMode;
		CompileShader();
	}
}

void UIShaderEditorWindow::ResetSelected()
{
	selMask = 0;
}

void UIShaderEditorWindow::SetPartTarget(ShaderPart* _part)
{
	if (_part)
	{
		ShaderNode* node = dynamic_cast<ShaderNode*>(_part);
		ShaderLeaf* leaf = dynamic_cast<ShaderLeaf*>(_part);
		if (node) isLeaf = false;
		if (leaf) isLeaf = true;
		currentPart = _part;
		Guizmo::SetTarget(currentPart);
		ResetSelected();
	}
}

void UIShaderEditorWindow::ReturnButton()
{
	if (!currentPart->GetParent()) return;
	if (ImGui::Button("back"))
	{
		SetPartTarget(currentPart->GetParent());
	}
}

void UIShaderEditorWindow::InputName()
{
	const char* tmpName = currentPart->name.c_str();
	char* newName = new char[128];
	strcpy(newName, tmpName);
	ImGui::InputText("name", newName, 128);
	currentPart->name = newName;
	delete[] newName;
}

void UIShaderEditorWindow::Save(std::string path, std::string fileName, void* data)
{
	std::string ext[1] = { ".scene" };
	std::string finalPath = path + fileName + std::string(UIResourceBrowser::HasRequestedExtension(fileName, ext, 1) ? "" : ".scene");
	shader->Save(finalPath, errorMsg, (ShaderNode*)data);
}

void UIShaderEditorWindow::Load(std::string path, std::string fileName, void* data)
{
	std::string finalPath = path + fileName;
	shader->Load(finalPath, errorMsg, (ShaderNode**)data);
	SetPartTarget(data != nullptr ? *(ShaderNode**)data : shader->root);

	compilationError = !shader->Recompile(errorMsg, errorCode);
	Material::SendData(shader);
	editor.SetText(errorCode);
}

void UIShaderEditorWindow::DisplayError()
{
	if (ImGui::Button("see generated code") || showingGeneratedCode)
	{
		showingGeneratedCode = true;

		ImGui::OpenPopup("generated code");
		ImGui::BeginPopup("generated code");

		editor.Render("generated code", ImVec2(800, 600));
		//ImGui::Text(errorCode.c_str());

		ImVec2 popUpSize, popUpPos, mousePos;
		popUpSize = ImGui::GetWindowSize();
		popUpPos = ImGui::GetWindowPos();
		mousePos = ImGui::GetMousePos();
		if (ImGui::IsAnyMouseDown() && !ImGui::IsMouseHoveringRect(popUpPos, ImVec2(popUpPos.x + popUpSize.x, popUpPos.y + popUpSize.y)))
		{
			showingGeneratedCode = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	if (compilationError)
	{
		ImGui::Text("\n\nCOMPILATION ERROR : \n%s\n\n", errorMsg.c_str());
	}
}

void UIShaderEditorWindow::TransformPoint()
{
	ImGui::Checkbox("transform point", &currentPart->transformPoint);
	if (currentPart->transformPoint)
	{
		bool isDynamic = currentPart->transform->IsDynamic();
		bool wasDynamic = isDynamic;

		ImGui::SameLine();
		ImGui::Checkbox("dynamic transform", &isDynamic);
		currentPart->transform->SetDynamic(isDynamic);

		if (!currentPart->transform->IsDynamic())
		{
			ImGui::Text("static transforms need to compile to see modifications");
		}

		if (wasDynamic != isDynamic) CompileShader();

		ImGui::DragFloat3("position (x/y/z)", (float*)&currentPart->transform->position, 0.1f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("rotation (x/y/z)", (float*)&currentPart->transform->rotation, 0.1f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat("scale (uniform)", (float*)&currentPart->transform->scale, 0.1f, 0, FLT_MAX);


		AnimateScale();
		AnimatePosition();
	}
	//AnimateDistance();
}

void UIShaderEditorWindow::AnimateScale()
{
	ImGui::Checkbox("animate scale", &currentPart->animateScale);
	if (currentPart->animateScale)
	{
		//ImGui::SameLine();

		const char* tmpFnc = currentPart->animatedScaleFunction.c_str();
		char* newFnc = new char[512];
		strcpy(newFnc, tmpFnc);
		ImGui::InputText("scale over progress [0, 1]", newFnc, 512);
		currentPart->animatedScaleFunction = newFnc;
		delete[] newFnc;
	}
}

void UIShaderEditorWindow::AnimatePosition()
{
	ImGui::Checkbox("animate position", &currentPart->animatePosition);
	if (currentPart->animatePosition)
	{
		const char* tmpFnc = currentPart->animatedPositionFunction.c_str();
		char* newFnc = new char[512];
		strcpy(newFnc, tmpFnc);
		ImGui::InputText("position over progress [0, 1]", newFnc, 512);
		currentPart->animatedPositionFunction = newFnc;
		delete[] newFnc;
	}
}

void UIShaderEditorWindow::ChooseSpaceRemap(ShaderFunction*& func)
{
	if (!func) return;
	int selected = func->model.ID;

	if (PickModel(FunctionType::SpaceRemap, selected))
	{
		auto it = ShaderFunction::models[FunctionType::SpaceRemap].begin();
		for (int i = 0; i < selected; ++i) ++it;
		currentPart->ReplaceSpaceRemap(func, new ShaderFunction(*(*it)));
	}
}

void UIShaderEditorWindow::RemapSpace()
{
	ImGui::Text("\n");
	ImGui::Text("Space Remapping functions : ");
	ImGui::BeginTabBar("SpaceRemap functions");

	if (currentPart->spaceRemaps.size() == 0)
	{
		if (ImGui::BeginTabItem("empty"))
		{
			ImGui::EndTabItem();
		}
	}
	else
	{
		int i = 1;
		auto beg = currentPart->spaceRemaps.begin();
		auto en = currentPart->spaceRemaps.end();
		for (auto func = beg; func != en; ++func)
		{
			bool stillOn = true;
			int flag = 0;

			if (i - 1 == SRToSwap + swapDir)
			{
				flag = ImGuiTabItemFlags_SetSelected;
				SRToSwap = -1;
				swapDir = 0;
			}
			
			if (ImGui::BeginTabItem(std::to_string(i).c_str(), &stillOn, flag))
			{
				bool isDynamic = (*func)->IsDynamic();
				ChooseSpaceRemap(*func);

				if (ImGui::Checkbox("dynamic data", &isDynamic))
				{
					(*func)->SetDynamic(isDynamic);
					CompileShader();
				}

				if (!(*func)->IsDynamic())
				{
					ImGui::Text("static data need to compile to see modifications");
				}

				ShaderFunctionDataSetter(*func);

				ImGui::EndTabItem();

				if (func != beg && ImGui::TabItemButton("<", ImGuiTabItemFlags_::ImGuiTabItemFlags_Trailing))
				{
					auto prev = func;
					std::swap(*func, *(--prev));
					SRToSwap = i - 1;
					swapDir = -1;
					CompileShader();
				}

				auto last = en;
				if (func != --last && ImGui::TabItemButton(">", ImGuiTabItemFlags_::ImGuiTabItemFlags_Trailing))
				{
					auto next = func;
					std::swap(*func, *(++next));
					SRToSwap = i - 1;
					swapDir = 1;
					CompileShader();
				}
			}

			if (!stillOn)
			{
				currentPart->RemoveSpaceRemap(*func);
				break;
			}

			++i;
		}
	}

	if (ImGui::TabItemButton("+", ImGuiTabItemFlags_::ImGuiTabItemFlags_Trailing))
	{
		if (ShaderFunction::models[FunctionType::SpaceRemap].size() > 0)
		{
			currentPart->AddSpaceRemap();
		}
	}

	ImGui::EndTabBar();
	ImGui::Text("\n");
}

void UIShaderEditorWindow::Elongate()
{
	ImGui::Checkbox("elongate", &currentPart->elongate);
	if (currentPart->elongate)
	{
		ImGui::DragFloat3("elongation (x/y/z)", (float*)&currentPart->elongation, 0.1f, 0, FLT_MAX);
	}
}

void UIShaderEditorWindow::Repeat()
{
	ImGui::Checkbox("repeat", &currentPart->repeat);
	if (currentPart->repeat)
	{
		ImGui::DragFloat3("repeat offset (x/y/z)", (float*)&currentPart->repeatOffset, 0.1f, 0, FLT_MAX);
	}
}

void UIShaderEditorWindow::RevolutionRepeat()
{
	ImGui::Checkbox("repeat by revolution", &currentPart->revolutionRepeat);
	if (currentPart->revolutionRepeat)
	{
		ImGui::DragFloat("repeat x times", &currentPart->revolutionTimesRepeat, 0.1f, 0, FLT_MAX);
	}
}

void UIShaderEditorWindow::RotateObject()
{
	ImGui::Checkbox("rotate over time", &currentPart->rotating);
	if (currentPart->rotating)
	{
		ImGui::DragFloat("rotate x times per second", &currentPart->rotatingSpeed, 0.1f, -FLT_MAX, FLT_MAX);
	}
}

void UIShaderEditorWindow::RoundShape()
{
	ImGui::Checkbox("round", &currentPart->round);
	if (currentPart->round || currentPart->hollow)
	{
		ImGui::SameLine();
		ImGui::Checkbox("hollow", &currentPart->hollow);
		ImGui::DragFloat("thickness", (float*)&currentPart->roundValue, 0.1f, 0, FLT_MAX);
	}
}

void UIShaderEditorWindow::ChooseMaterial()
{
	int oldSelected = currentPart->matID;
	int selected = currentPart->matID + 1;

	ImGui::Combo("Select material", &selected,
		[](void* data, int idx, const char** out_text)
		{
			idx -= 1;

			if (idx < 0)
			{
				*out_text = const_cast<const char*>(UIShaderEditorWindow::noMatText);
				return true;
			}

			std::list<Material*>* matList = (std::list<Material*>*)data;
			if (idx >= 0 && idx < matList->size())
			{
				auto it = matList->begin();
				for (int i = 0; i < idx; ++i) ++it;
				*out_text = (*it)->name.c_str();
				return true;
			}
			else
			{
				return false;
			}
		}
	, &Material::materials, Material::materials.size() + 1);

	if (selected - 1 != oldSelected)
	{
		currentPart->matID = selected - 1;
		auto it = Material::materials.begin();

		compilationError = !shader->Recompile(errorMsg, errorCode);
		editor.SetText(errorCode);
	}
}

void UIShaderEditorWindow::UpdateRenderFrameRate()
{
	renderFPSCounter.Update();
}

void UIShaderEditorWindow::WindowBody()
{
	UIWindow::WindowBody();

	if (ImGui::IsWindowFocused())
	{
		Guizmo::SetTarget(currentPart);
	}

	mainFPSCounter.Update();
	ImGui::Text("frame time %f s | frame rate %f", mainFPSCounter.meanFrameTime, 1.0f / mainFPSCounter.meanFrameTime);
	ImGui::Text("Render time %f s | render rate %f", renderFPSCounter.meanFrameTime, 1.0f / renderFPSCounter.meanFrameTime);

	ReturnButton();
	if (currentPart->GetParent()) ImGui::SameLine();
	CompileButton();
	DisplayError();
	InputName();
	ImGui::Text("\n\nShader part data : \n\n");
	TransformPoint();
	RevolutionRepeat();
	RotateObject();
	Repeat();
	Elongate();
	RoundShape();
	ImGui::DragFloat("smooth mix", &currentPart->smooth, 0.1f, 0, FLT_MAX);
	ImGui::DragFloat("bias", &currentPart->bias, 0.1f, 0, 1.0f);
	//ImGui::SameLine();
	BlendModeButton();
	if (ImGui::Checkbox("Emit Light", &currentPart->emittingLight))
	{
		CompileShader();
	}
	RemapSpace();

	if (isLeaf)
	{
		LeafBody();
	}
	else
	{
		NodeBody();
	}

	ChooseMaterial();
}
