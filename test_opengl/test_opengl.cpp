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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "UIPlayer.h"

#define BALL_NUM 1

DeclareDelegate(OnWheelScroll, GLFWwindow*, double, double)
OnWheelScroll onWheelScroll;

void OnWheelScrolled(GLFWwindow* _window, double _x, double _y)
{
	onWheelScroll(_window, _x, _y);
}

using namespace std;

int main(int argc, char* argv[])
{
	UIResourceBrowser::SetApplicationDirectory(UIResourceBrowser::RemoveFileNameFromPath(argv[0]));
	Light::Init();
	Transform::Init();
	ShaderLeaf::Init();
	Guizmo::Init();
	ShaderFunction::Init();
	srand(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	rand();
	rand();
	rand();
	rand();
	rand();

	GLFWwindow* window = ViewportManager::CreateWindow();
	if (!window)
	{
		std::cout << "program stopped, couldn't create opengl window" << std::endl;
		return -1;
	}

	// init imgui
	const char* glsl_version = "#version 130";
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGuizmo::Enable(true);
	ShaderGenerator testShader/*("Shaders/VertexShader.shader", "Shaders/FragmentShader.shader")*/;
	RenderPlane* quad = new RenderPlane(&testShader);
	ViewportManager::Init(&testShader, quad);
	Time::Init();

	{
		std::string tmpCode;
		std::string tmpMsg;
		testShader.Recompile(tmpMsg, tmpCode);
	}

	UIShaderEditorWindow* sceneEditor = new UIShaderEditorWindow("scene editor", &testShader);
	UILightingWindow* lightingEditor = new UILightingWindow("lighting editor", testShader.lighting);
	UIMaterialInspector* materialEditor = new UIMaterialInspector("material editor");
	UIShaderFunctionEditor* CustomPrimitiveEditor = new UIShaderFunctionEditor("custom primitive editor");
	UIDebug* debugWindow = new UIDebug("Debug", &testShader);
	UIGlobalParam* globalParamWindow = new UIGlobalParam("Global Parameters", &testShader);
	UIPlayer* playerWindow = new UIPlayer("Player", quad);
	materialEditor->shader = &testShader;
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
	onWheelScroll.Bind(glfwSetScrollCallback(window, OnWheelScrolled));

	// test materials

	bool qwerty = false;
	bool antiRepeat = false;
	double mouseX, lastX;
	double mouseY, lastY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	bool HasUpdatedRender = false;
	bool wasRightClicking = false;

	if (argc == 2)
	{
		sceneEditor->Load(argv[1], "", nullptr);
	}

	while (!glfwWindowShouldClose(window))
	{
		Vector3 camMove;

		lastY = mouseY;
		lastX = mouseX;

		Time::Update();

		bool shouldRepeat = false;

		//Event
		glfwPollEvents();

		/*if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}
		else
		{
			shouldRepeat = true;
		}*/

		if (glfwGetMouseButton(window, 0) && ImGui::IsMouseDragging(0.1))
		{
			float width, height;
			double mx, my;
			float border = 1;
			ViewportManager::GetScreenSize(width, height);
			glfwGetCursorPos(window, &mx, &my);
			bool mouseTeleported = false;

			if (mx >= width - border)
			{
				mx = border + 1;
				mouseTeleported = true;
			}
			else if (mx <= border)
			{
				mx = width - (border + 1);
				mouseTeleported = true;
			}

			if (my >= height - border)
			{
				my = border + 1;
				mouseTeleported = true;
			}
			else if (my <= border)
			{
				my = height - (border + 1);
				mouseTeleported = true;
			}

			if (mouseTeleported)
			{
				ImGui::GetIO().MousePosPrev = ImVec2(mx, my);
				ImGui::GetIO().MousePos = ImVec2(mx, my);
				glfwSetCursorPos(window, mx, my);
			}
		}

		if (!ImGui::GetIO().WantTextInput && !ImGui::GetIO().WantCaptureKeyboard)
		{
			shouldRepeat = false;
			if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					Guizmo::SetOperation((int)ImGuizmo::TRANSLATE);
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					qwerty = !qwerty;
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					sceneEditor->CompileShader();
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					Guizmo::SetOperation((int)ImGuizmo::ROTATE);
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					Guizmo::SetOperation((int)ImGuizmo::SCALE);
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					Guizmo::SetWorld(!Guizmo::IsWorld());
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					ViewportManager::MaximizeSceneView(!ViewportManager::IsSceneViewMaximized());
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					Guizmo::SetVisible(!Guizmo::IsVisible());
					antiRepeat = true;
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			{
				if (!antiRepeat)
				{
					Time::playing = !Time::playing;
					antiRepeat = true;
				}
			}
			else
			{
				shouldRepeat = true;
			}
		}

		if (shouldRepeat)
		{
			antiRepeat = false;
		}

		glfwGetCursorPos(window, &mouseX, &mouseY);

		if (glfwGetMouseButton(window, 1) && (wasRightClicking || ViewportManager::IsQuadContaining(ImVec2(mouseX, mouseY))))
		{
			if (!wasRightClicking) onWheelScroll.Bind(Camera::SetCameraSpeed);
			wasRightClicking = true;
			float width, height;
			ViewportManager::GetScreenSize(width, height);
			glfwSetCursorPos(window, width / 2, height / 2);
			camera.LookAt(camera.GetPosition() + camera.GetForward() + ((mouseX - lastX) * camera.GetRight() - (mouseY - lastY) * camera.GetUp()) / 100.0f);
			glfwGetCursorPos(window, &mouseX, &mouseY);

			if (qwerty)
			{
				if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
					camMove.z += 1;
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					camMove.z += -1;
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					camMove.x += 1;
				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
					camMove.x += -1;
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
					camMove.y += -1;
				if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
					camMove.y += 1;
			}
			else
			{
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
					camMove.z += 1;
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					camMove.z += -1;
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					camMove.x += 1;
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
					camMove.x += -1;
				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
					camMove.y += -1;
				if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
					camMove.y += 1;
			}

			if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
			{
				camera.SetPosition(Vector3());
				camera.LookAt(Vector3(0, 0, 1));
				Camera::SetCameraSpeed(window, 0, -10000000);
			}

		}
		else
		{
			wasRightClicking = false;
			onWheelScroll.Unbind(Camera::SetCameraSpeed);
		}

		/*static double truc = 0;
		if (Time::fullTime - truc > 1)
		{
			truc = Time::fullTime;
			ViewportManager::LoadLayoutOneStep();
		}*/

		// new imgui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ViewportManager::Update();
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		// Update
		menuBar.Display();
		UIWindow::DisplayUI();
		ImGuizmo::BeginFrame();

		camera.Move(camMove, Time::GetFrameTime());
		//camera.ApplyToShader(&testShader);
		Guizmo::Update(&camera);

		//testShader.setFloat("_Metallic", 0.0f);
		//testShader.setFloat("_Metallic", 0.8f);
		/*testShader.setFloat("_Gloss", 0.5f);
		testShader.setFloat("_Shine", 2.5f);*/
		if (HasUpdatedRender)
		{
			camera.ApplyToShader(&testShader);
			testShader.setDouble("timeSinceLaunch", Time::GetTime());
			sceneEditor->UpdateRenderFrameRate();
		}

		testShader.lighting->UpdateData();
		Transform::SendData(&testShader);
		ShaderLeaf::SendData(&testShader);
		ShaderFunction::SendData(&testShader);
		Material::SendData(&testShader);

		//Display

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		testShader.use();
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
