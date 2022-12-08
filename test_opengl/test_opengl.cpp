#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "math.h"
#include "Quad.h"
#include "ShaderLeaf.h"
#include "ShaderGenerator.h"
#include "UIWindow.h"
#include "UIPopUp.h"
#include "UIShaderTextEditorWindow.h"
#include "UIShaderEditorWindow.h"
#include "UILightingWindow.h"
#include "Camera.h"
#include "Lighting.h"
#include "Light.h"
#include "ViewportManager.h"
#include "Transform.h"
#include "ImGuizmo/ImGuizmo.h"
#include "Guizmo.h"
#include "UIMaterialInspector.h"
#include "UICustomPrimitiveEditor.h"
#include "Material.h"
#include "BalazsJakoTextEditor/TextEditor.h"
#include <chrono>
#include "UIDebug.h"
#include "CustomPrimitive.h"
#include "RenderPlane.h"
#include "UIGlobalParam.h"
#include "UIResourceBrowser.h"
#include "Time.h"
#include "MenuBar.h"
#include "Input/Input.h"
#include "BoundingVolume.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "UIPlayer.h"

#define BALL_NUM 1

using namespace std;

int main(int argc, char* argv[])
{
	srand(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	rand();
	rand();
	rand();
	rand();
	rand();

	glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
	GLFWwindow* window = ViewportManager::CreateWindow();
	if (!window)
	{
		std::cout << "program stopped, couldn't create opengl window" << std::endl;
		return -1;
	}
	Input::Init();
	Input::GetGlobalInput(0);
	UIResourceBrowser::SetApplicationDirectory(UIResourceBrowser::RemoveFileNameFromPath(argv[0]));
	Light::Init();
	Transform::Init();
	ShaderLeaf::Init();
	Guizmo::Init();
	ShaderFunction::Init();

	// init imgui
	const char* glsl_version = "#version 130";
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGuizmo::Enable(true);
	ShaderGenerator shader;
	RenderPlane* quad = new RenderPlane(&shader);
	ViewportManager::Init(&shader, quad);
	Time::Init();

	{
		std::string tmpCode;
		std::string tmpMsg;
		shader.Recompile(tmpMsg, tmpCode);
	}

	UIShaderEditorWindow* sceneEditor = new UIShaderEditorWindow("scene editor", &shader);
	UILightingWindow* lightingEditor = new UILightingWindow("lighting editor", shader.lighting);
	UIMaterialInspector* materialEditor = new UIMaterialInspector("material editor");
	UIShaderFunctionEditor* CustomPrimitiveEditor = new UIShaderFunctionEditor("custom primitive editor");
	UIDebug* debugWindow = new UIDebug("Debug", &shader);
	UIGlobalParam* globalParamWindow = new UIGlobalParam("Global Parameters", &shader);
	UIPlayer* playerWindow = new UIPlayer("Player", quad);
	materialEditor->shader = &shader;
	float width, height;
	ViewportManager::GetScreenSize(width, height);

	if (!ViewportManager::LoadLayout())
	{
		ViewportManager::Snap(sceneEditor, ImVec2(width / 5.0f, height / 2.0f));
		ViewportManager::Snap(CustomPrimitiveEditor, sceneEditor);
		ViewportManager::Snap(lightingEditor, ImVec2(3.0f * width / 4.0f, height / 2.0f));
		ViewportManager::Snap(materialEditor, lightingEditor);
		ViewportManager::Snap(globalParamWindow, ImVec2(0.5f * width, 4.5f * height / 5.0f));
		ViewportManager::Snap(playerWindow, globalParamWindow);
		ViewportManager::Snap(debugWindow, globalParamWindow);
	}

	MenuBar menuBar(sceneEditor);
	Camera camera;

	bool HasUpdatedRender = false;

	if (argc == 2)
	{
		sceneEditor->Load(argv[1], "", nullptr);
	}

	{
		Input::BindSet bs = Input::BindSet("bindsetTest.bs");
		Input::GetGlobalInput(0).ApplyBindSet(bs);
	}
	bool allowInput = true;
	while (!glfwWindowShouldClose(window))
	{
		//Event
		Input::PollEvents();

		// new imgui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		Input::GetGlobalInput(0).AllowInput(allowInput);
		Time::Update();
		ViewportManager::Update();

		menuBar.Display();
		UIWindow::DisplayUI();
		allowInput = !ImGui::GetIO().WantTextInput && !ImGui::GetIO().WantCaptureKeyboard;
		ImGuizmo::BeginFrame();
		camera.Move(Time::GetFrameTime());
		Guizmo::Update(&camera);
		if (HasUpdatedRender)
		{
			camera.ApplyToShader(&shader);
			shader.setDouble("timeSinceLaunch", Time::GetTime());
			sceneEditor->UpdateRenderFrameRate();
		}

		shader.lighting->UpdateData();
		Transform::SendData(&shader);
		ShaderLeaf::SendData(&shader);
		ShaderFunction::SendData(&shader);
		Material::SendData(&shader);

		//Display
		/*glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);*/

		shader.use();
		HasUpdatedRender = quad->Draw();

		//ViewportManager::DebugDrawEdges();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	ViewportManager::SaveLayout();

	return 0;
}
