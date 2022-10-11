#include "ViewportManager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image/stb_image.h"
#include "UIResourceBrowser.h"
#include "UIHost.h"

#define Min(val1, val2) ((val2) > (val1) ? (val1) : (val2))
#define Max(val1, val2) ((val2) > (val1) ? (val2) : (val1))
#define Clamp(var, minVal, maxVal) (var = Min(maxVal, Max(var, minVal)))

enum EdgeOrientation
{
	Horizontal,
	Vertical
};


//---- EDGE ----//

typedef struct Edge
{
	EdgeOrientation orientation;
	float position;
	Edge* top;
	Edge* bot;
	std::list<ViewportSnapSection*> right;
	std::list<ViewportSnapSection*> left;

	void GetPositions(Vector3& outTop, Vector3& outBot);
	void NotifySectionDestruction(ViewportSnapSection* section, Edge* replacementEdge);

private:


	void RebindSections(Edge* replacementEdge);
} Edge;

void Edge::GetPositions(Vector3& outTop, Vector3& outBot)
{
	float width, height;
	ViewportManager::GetScreenSize(width, height);
	if (orientation == EdgeOrientation::Horizontal)
	{
		outTop = Vector3((top) ? top->position * width : 0, position * height, 0);
		outBot = Vector3((bot) ? bot->position * width : width, position * height, 0);
	}
	else
	{
		outTop = Vector3(position * width, (top) ? top->position * height : 0, 0);
		outBot = Vector3(position * width, (bot) ? bot->position * height : height, 0);
	}
}

void Edge::NotifySectionDestruction(ViewportSnapSection* section, Edge* replacementEdge)
{
	right.remove(section);
	left.remove(section);

	if (right.size() == 0 || left.size() == 0)
	{
		RebindSections(replacementEdge);
		ViewportManager::RemoveEdge(this);
	}
}

void Edge::RebindSections(Edge* replacementEdge)
{
	for (auto it = right.begin(); it != right.end(); ++it)
	{
		ViewportSnapSection* section = *it;

		if (orientation == EdgeOrientation::Horizontal)
		{
			section->bot = replacementEdge;
		}
		else
		{
			section->left = replacementEdge;
		}
		if (replacementEdge) replacementEdge->right.push_back(section);
	}
	for (auto it = left.begin(); it != left.end(); ++it)
	{
		ViewportSnapSection* section = *it;

		if (orientation == EdgeOrientation::Horizontal)
		{
			section->top = replacementEdge;
		}
		else
		{
			section->right = replacementEdge;
		}
		if (replacementEdge) replacementEdge->left.push_back(section);
	}
	ViewportManager::RebindEdges(this, replacementEdge);
}

//---- VIEWPORT MANAGER ----//

Shader* ViewportManager::shader;
RenderPlane* ViewportManager::sceneQuad;
std::list<Edge*> ViewportManager::edges;
std::list<ViewportSnapSection*> ViewportManager::sections;
float ViewportManager::screenWidth, ViewportManager::screenHeight;
float ViewportManager::screenResolutionScale;
GLFWwindow* ViewportManager::window;
bool ViewportManager::maximizeSceneView;

void ViewportManager::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	if (width && height)
	{
		screenWidth = width;
		screenHeight = height;
		glViewport(0, 0, width, height);
		AdjustQuad();
	}
}

void ViewportManager::Init(Shader* _shader, RenderPlane* _quad)
{
	shader = _shader;
	sceneQuad = _quad;
	maximizeSceneView = false;
	screenResolutionScale = 1.0f;

	sections.push_back(new ViewportSnapSection());

	WindowSizeCallback(window, screenWidth, screenHeight);
}

GLFWwindow* ViewportManager::CreateWindow()
{
	// init open gl
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSwapInterval(0);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	screenWidth = 1110;
	screenHeight = 600;

	// create window
	GLFWimage icons[1];
	icons[0].pixels = stbi_load(std::string(UIResourceBrowser::GetApplicationDirectory() + "NoPoly.png").c_str(), &icons[0].width, &icons[0].height, 0, 4);
	window = glfwCreateWindow(1110, 600, "NoPoly", NULL, NULL);
	glfwSetWindowIcon(window, 1, icons);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	// init glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	// setting gl viewport
	glViewport(0, 0, 1110, 600);

	// setting callbacks
	glfwSetFramebufferSizeCallback(window, WindowSizeCallback);

	return window;
}

void ViewportManager::RebindEdges(Edge* oldEdge, Edge* replacementEdge)
{
	for (auto it = edges.begin(); it != edges.end(); ++it)
	{
		Edge* edge = *it;
		if (edge->bot == oldEdge)
		{
			edge->bot = replacementEdge;
		}
		else if (edge->top == oldEdge)
		{
			edge->top = replacementEdge;
		}
	}
}

ViewportSnapSection* ViewportManager::AddSection(ImVec2 position)
{
	if (position.x < 0 || position.x > screenWidth || position.y < 0 || position.y > screenHeight)
	{
		std::cout << "tried to set a viewport separation outside the window" << std::endl;
		return nullptr;
	}

	ViewportSnapSectionQuadrant quadrant;
	ViewportSnapSection* section = GetSectionAndQuadrant(ImVec2(position.x, position.y), quadrant);
	EdgeOrientation orientation = (quadrant == right || quadrant == left) ? Vertical : Horizontal;
	Edge* newEdge = new Edge();
	newEdge->orientation = orientation;
	newEdge->position = (orientation == EdgeOrientation::Horizontal) ? (position.y / screenHeight) : (position.x / screenWidth);

	Edge* top = nullptr;
	Edge* bot = nullptr;
	float topProxi = 10000.0f, botProxi = 10000.0f;

	for (auto it = edges.begin(); it != edges.end(); ++it)
	{
		Edge* edge = (*it);
		if (edge->orientation != orientation)
		{
			Vector3 topPos;
			Vector3 botPos;
			edge->GetPositions(topPos, botPos);

			float dotTop = Vector3::Dot(botPos - topPos, Vector3(position.x, position.y, 0) - topPos);
			float dotBot = Vector3::Dot(topPos - botPos, Vector3(position.x, position.y, 0) - botPos);
			float proximity = (orientation == EdgeOrientation::Horizontal) ? (edge->position - position.x / screenWidth) : (edge->position - position.y / screenHeight);

			if (dotTop < 0.0f || dotBot < 0.0f) continue;

			if (proximity > 0 && proximity < botProxi)
			{
				botProxi = proximity;
				bot = edge;
			}
			else if (proximity < 0 && fabs(proximity) < topProxi)
			{
				topProxi = fabs(proximity);
				top = edge;
			}
			else if (proximity == 0)
			{
				std::cout << "tried to create a viewport separation on another" << std::endl;
				delete newEdge;
				return nullptr;
			}
		}
	}
	newEdge->top = top;
	newEdge->bot = bot;
	edges.push_back(newEdge);

	ViewportSnapSection* newSection = new ViewportSnapSection();
	sections.push_back(newSection);

	switch (quadrant)
	{
	case ViewportSnapSectionQuadrant::top:

		newSection->top = section->top;
		newSection->left = section->left;
		newSection->right = section->right;
		newSection->bot = newEdge;

		if (newSection->top) newSection->top->left.push_back(newSection);
		if (newSection->left) newSection->left->right.push_back(newSection);
		if (newSection->right) newSection->right->left.push_back(newSection);
		if (newSection->bot) newSection->bot->right.push_back(newSection);

		if (section->top) section->top->left.remove(section);
		section->top = newEdge;
		if (section->top) section->top->left.push_back(section);

		break;
	case ViewportSnapSectionQuadrant::bot:

		newSection->bot = section->bot;
		newSection->left = section->left;
		newSection->right = section->right;
		newSection->top = newEdge;

		if (newSection->top) newSection->top->left.push_back(newSection);
		if (newSection->left) newSection->left->right.push_back(newSection);
		if (newSection->right) newSection->right->left.push_back(newSection);
		if (newSection->bot) newSection->bot->right.push_back(newSection);

		if (section->bot) section->bot->right.remove(section);
		section->bot = newEdge;
		if (section->bot) section->bot->right.push_back(section);

		break;
	case ViewportSnapSectionQuadrant::left:
		newSection->top = section->top;
		newSection->left = section->left;
		newSection->bot = section->bot;
		newSection->right = newEdge;

		if (newSection->top) newSection->top->left.push_back(newSection);
		if (newSection->left) newSection->left->right.push_back(newSection);
		if (newSection->right) newSection->right->left.push_back(newSection);
		if (newSection->bot) newSection->bot->right.push_back(newSection);

		if (section->left) section->left->right.remove(section);
		section->left = newEdge;
		if (section->left) section->left->right.push_back(section);

		break;
	case ViewportSnapSectionQuadrant::right:
		newSection->top = section->top;
		newSection->bot = section->bot;
		newSection->right = section->right;
		newSection->left = newEdge;

		if (newSection->top) newSection->top->left.push_back(newSection);
		if (newSection->left) newSection->left->right.push_back(newSection);
		if (newSection->right) newSection->right->left.push_back(newSection);
		if (newSection->bot) newSection->bot->right.push_back(newSection);

		if (section->right) section->right->left.remove(section);
		section->right = newEdge;
		if (section->right) section->right->left.push_back(section);
		break;
	}

	AdjustQuad();

	return newSection;
}

void ViewportManager::AdjustQuad()
{
	ImVec2 pos, size;
	Vector3 pos3, size3;
	ViewportSnapSection* section = *sections.begin();

	if (maximizeSceneView)
	{
		pos = ImVec2(0, 0);
		size = ImVec2(screenWidth, screenHeight);
	}
	else
	{
		section->GetSizeAndPosition(size, pos);
	}

	pos3 = ((Vector3(pos.x, pos.y, 1) / Vector3(screenWidth, screenHeight, 1)) * 2.0f - Vector3(1.0f, 1.0f, 3.0f));
	size3 = (Vector3(size.x, size.y, 1) / Vector3(screenWidth, screenHeight, 1)) * 2.0f;
	
	sceneQuad->SetPosAndSize(pos3, size3, Vector3(size.x, size.y, 1) * screenResolutionScale);

	shader->setInt("screenWidth", size.x);
	shader->setInt("screenHeight", size.y);
}

void ViewportManager::RemoveEdge(Edge* edge)
{
	edges.remove(edge);
	delete edge;
	AdjustQuad();
}

void ViewportManager::RemoveSection(ViewportSnapSection* section)
{
	if (section == nullptr) return;
	sections.remove(section);
	section->Destroy();
	AdjustQuad();
}

ViewportSnapSection* ViewportManager::GetSectionAndQuadrant(ImVec2 position, ViewportSnapSectionQuadrant& outQuadrant, bool noCenter)
{
	ViewportSnapSectionQuadrant quadrant;
	for (auto it = sections.begin(); it != sections.end(); ++it)
	{
		ViewportSnapSection* tmpSection = *it;

		quadrant = tmpSection->Contains(position, noCenter || it == sections.begin());

		if (quadrant != outside)
		{
			outQuadrant = quadrant;
			return tmpSection;
		}
	}

	return nullptr;
}

void ViewportManager::Snap(UIWindow* window, ImVec2 position)
{
	if (position.x < 0 || position.x > screenWidth || position.y < 0 || position.y > screenHeight)
	{
		std::cout << "tried to snap outside the window" << std::endl;
		return;
	}
	ImVec2 pos, size, newPos;
	Vector3 pos3, size3, newPos3;
	ViewportSnapSectionQuadrant quadrant;
	ViewportSnapSection* newSection = nullptr;
	ViewportSnapSection* section = GetSectionAndQuadrant(position, quadrant);
	if (!section || section == window->snapSection) return;
	section->GetSizeAndPosition(size, pos);
	pos3 = Vector3(pos.x, pos.y, 0);
	size3 = Vector3(size.x, size.y, 0);
	newPos3 = pos3 + size3 / 2.0f;

	switch (quadrant)
	{
	case top:
		newPos3 += Vector3(0, -size3.y / 4.0f, 0);
		break;
	case bot:
		newPos3 += Vector3(0, size3.y / 4.0f, 0);
		break;
	case left:
		newPos3 += Vector3(-size3.x / 4.0f, 0, 0);
		break;
	case right:
		newPos3 += Vector3(size3.x / 4.0f, 0, 0);
		break;
	}

	if (quadrant != center || section == *sections.begin())
	{
		newSection = AddSection(ImVec2(newPos3.x, newPos3.y));

		if (!newSection)
		{
			std::cout << "failed to create new section" << std::endl;
			return;
		}

		if (window->snapSection)
		{
			ViewportManager::RemoveSection(window->snapSection);
		}

		window->snapSection = newSection;
	}
	else
	{
		UIWindow* windowInSec = GetUIWindowInSection(section);
		Snap(window, windowInSec);
	}
	AdjustQuad();
}

void ViewportManager::Snap(UIWindow* window, UIWindow* parentWindow)
{
	if (parentWindow)
	{
		UIHost* host = nullptr;
		if (parentWindow->snapSection)
		{
			host = new UIHost("Host Window");
			((UIWindow*)host)->snapSection = parentWindow->snapSection;
			host->AddChild(parentWindow);
		}
		else
		{
			host = (UIHost*)parentWindow->host;
		}
		if (host == nullptr) return;
		ViewportManager::RemoveSection(window->snapSection);
		host->AddChild(window);
	}
}

void ViewportManager::DebugDrawEdges()
{
	ViewportSnapSection* quad0 = *sections.begin();
	int i = 0;
	std::cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" << std::endl;
	std::cout << "quad 0 section top = " << quad0->top << " bot = " << quad0->bot << " left = " << quad0->left << " right = " << quad0->right << std::endl;
	for (auto it = edges.begin(); it != edges.end(); ++it)
	{
		Edge* edge = *it;
		Vector3 top;
		Vector3 bot;
		edge->GetPositions(top, bot);
		std::cout << "-------------------------------------------------------------------------------------------" << std::endl;
		std::cout << edge << std::endl;
		std::cout << "top = " << top.x << " " << top.y << "\n" << "bot = " << bot.x << " " << bot.y << std::endl;
		std::cout << "right " << edge->right.size() << " left " << edge->left.size() << std::endl;
		ImGui::Begin((std::string("test edge ") + std::to_string(i)).c_str());
		ImGui::SetWindowPos(ImVec2(top.x, top.y));
		ImGui::SetWindowSize(ImVec2(bot.x - top.x, bot.y - top.y));
		ImGui::End();
		++i;
	}
}

void ViewportManager::GetScreenSize(float& outWidth, float& outHeight)
{
	outWidth = screenWidth;
	outHeight = screenHeight;
}

float ViewportManager::FixNextPosition(Edge* edge, float nextPosition)
{
	Vector3 topPos;
	Vector3 botPos;
	edge->GetPositions(topPos, botPos);
	
	float topProxi = edge->position;
	float botProxi = 1.0f - edge->position;
	float refSize = (edge->orientation == EdgeOrientation::Horizontal) ? screenHeight : screenWidth;

	for (auto it = edges.begin(); it != edges.end(); ++it)
	{
		Edge* tmpEdge = (*it);
		if (tmpEdge == edge) continue;
		if (tmpEdge->orientation == edge->orientation)
		{
			Vector3 tmpTopPos;
			Vector3 tmpBotPos;
			tmpEdge->GetPositions(tmpTopPos, tmpBotPos);

			float topTop = Vector3::Dot((tmpBotPos - tmpTopPos), (topPos - tmpTopPos)) / powf((tmpBotPos - tmpTopPos).Length(), 2.0f);
			float topBot = Vector3::Dot((tmpBotPos - tmpTopPos), (botPos - tmpTopPos)) / powf((tmpBotPos - tmpTopPos).Length(), 2.0f);
			float botTop = Vector3::Dot((tmpTopPos - tmpBotPos), (topPos - tmpBotPos)) / powf((tmpTopPos - tmpBotPos).Length(), 2.0f);
			float botBot = Vector3::Dot((tmpTopPos - tmpBotPos), (botPos - tmpBotPos)) / powf((tmpTopPos - tmpBotPos).Length(), 2.0f);

			bool isTopOutLeft = (topTop < 0.01f && botTop > 0.99f);
			bool isTopOutRight = (topTop > 0.99f && botTop < 0.01f);
			bool isBotOutLeft = (topBot < 0.01f && botBot > 0.99f);
			bool isBotOutRight = (topBot > 0.99f && botBot < 0.01f);

			if ((isTopOutLeft && isBotOutLeft) || (isTopOutRight && isBotOutRight)) continue;

			float proximity = tmpEdge->position - edge->position;

			/*if (fabs(proximity) < 50.0f / refSize)
			{
				std::cout << "tried to move a viewport separation on another" << std::endl;
				return edge->position;
			}
			else */if (proximity > 0 && proximity < botProxi)
			{
				botProxi = proximity;
			}
			else if (proximity < 0 && -proximity < topProxi)
			{
				topProxi = -proximity;
			}
		}
	}

	const float boundOffset = 50.0f / refSize;
	ImVec2 bounds = ImVec2(
		edge->position - topProxi + boundOffset,
		edge->position + botProxi - boundOffset
	);

	if (bounds.x > bounds.y) return edge->position;

	Clamp(nextPosition, bounds.x, bounds.y);

	if (nextPosition >= bounds.x && nextPosition <= bounds.y)
		return nextPosition;
	else													
		return edge->position;
}

void ViewportManager::MaximizeSceneView(bool _maximize)
{
	maximizeSceneView = _maximize;
	AdjustQuad();
}

bool ViewportManager::IsSceneViewMaximized()
{
	return maximizeSceneView;
}

void ViewportManager::GetSceneViewPosAndSize(ImVec2& outSize, ImVec2& outPos)
{
	if (maximizeSceneView)
	{
		outPos = ImVec2(0, 0);
		outSize = ImVec2(screenWidth, screenHeight);
	}
	else
	{
		ViewportSnapSection* quadSection = sections.front();

		if (quadSection)
		{
			quadSection->GetSizeAndPosition(outSize, outPos);
		}
	}
}

UIWindow* ViewportManager::GetUIWindowInSection(ViewportSnapSection* _section)
{
	UIWindow* uiWindow = nullptr;

	for (UIWindow*& tmpWindow : UIWindow::windows)
	{
		if (tmpWindow->snapSection == _section)
		{
			uiWindow = tmpWindow;
			break;
		}
	}

	return uiWindow;
}

//---- VIEWPORT SNAP SECTIONS ----//

void ViewportSnapSection::GetSizeAndPosition(ImVec2& outSize, ImVec2& outPos) const
{
	float width, height;
	ViewportManager::GetScreenSize(width, height);
	float lPos = (left ? left->position : 0) * width;
	float rPos = (right ? right->position : 1.0f) * width;
	float tPos = (top ? top->position : 20.0f / height) * height;
	float bPos = (bot ? bot->position : 1.0f) * height;
	outSize = ImVec2(rPos - lPos, bPos - tPos);
	outPos = ImVec2(lPos, tPos);
}

ViewportSnapSectionQuadrant ViewportSnapSection::Contains(const ImVec2& _position, bool noCenter) const
{
	ImVec2 position, size;
	Vector3 position3, size3, inPosition3;
	GetSizeAndPosition(size, position);

	position3 = Vector3(position.x, position.y, 0.0f);
	size3 = Vector3(size.x, size.y, 1.0f);
	inPosition3 = Vector3(_position.x, _position.y, 0.0f);

	if (_position.x > position.x && _position.x < position.x + size.x
		&& _position.y > position.y && _position.y < position.y + size.y)
	{
		Vector3 center = position3 + size3 / 2.0f;
		Vector3 dirToPoint = (inPosition3 - center) / size3;
		dirToPoint.z = 0;

		if (!noCenter && dirToPoint.Length() < 0.15f) return ViewportSnapSectionQuadrant::center;

		if (fabs(dirToPoint.x) > fabs(dirToPoint.y))
		{
			return dirToPoint.x > 0 ? ViewportSnapSectionQuadrant::right : ViewportSnapSectionQuadrant::left;
		}
		else
		{
			return dirToPoint.y > 0 ? ViewportSnapSectionQuadrant::bot : ViewportSnapSectionQuadrant::top;
		}
	}

	return outside;
}

ViewportSnapSectionQuadrant ViewportSnapSection::GetDirectionFromSectionCenter(const ImVec2& _position) const
{
	ImVec2 position, size;
	Vector3 position3, size3, inPosition3;
	GetSizeAndPosition(size, position);

	position3 = Vector3(position.x, position.y, 0.0f);
	size3 = Vector3(size.x, size.y, 0.0f);
	inPosition3 = Vector3(_position.x, _position.y, 0.0f);

	Vector3 center = position3 + size3 / 2.0f;
	Vector3 dirToPoint = (inPosition3 - center) / size3;

	if (fabs(dirToPoint.x) > fabs(dirToPoint.y))
	{
		return dirToPoint.x > 0 ? ViewportSnapSectionQuadrant::right : ViewportSnapSectionQuadrant::left;
	}
	else
	{
		return dirToPoint.y > 0 ? ViewportSnapSectionQuadrant::bot : ViewportSnapSectionQuadrant::top;
	}
}

void ViewportSnapSection::PushSide(ViewportSnapSectionQuadrant side, float value)
{
	Edge* edgeToPush = nullptr;
	float width, height;
	ViewportManager::GetScreenSize(width, height);
	float refSize = (side == ViewportSnapSectionQuadrant::left || side == ViewportSnapSectionQuadrant::right) ? width : height;

	switch (side)
	{
	case ViewportSnapSectionQuadrant::top:
		value = -value;
		edgeToPush = top;
		break;
	case ViewportSnapSectionQuadrant::bot:
		edgeToPush = bot;
		break;
	case ViewportSnapSectionQuadrant::left:
		value = -value;
		edgeToPush = left;
		break;
	case ViewportSnapSectionQuadrant::right:
		edgeToPush = right;
		break;
	}

	if (edgeToPush)
	{
		float nextPosition = edgeToPush->position + value / refSize;

		edgeToPush->position = ViewportManager::FixNextPosition(edgeToPush, nextPosition);
		ViewportManager::AdjustQuad();
	}
}

ViewportSnapSection::ViewportSnapSection()
{
	left = nullptr;
	right = nullptr;
	top = nullptr;
	bot = nullptr;
}

ViewportSnapSection::~ViewportSnapSection()
{

}

void ViewportSnapSection::Destroy()
{
	if (left) left->NotifySectionDestruction(this, right);
	if (right) right->NotifySectionDestruction(this, left);

	if (top) top->NotifySectionDestruction(this, bot);
	if (bot) bot->NotifySectionDestruction(this, top);

	delete this;
}
