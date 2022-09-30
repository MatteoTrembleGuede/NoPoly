#include "UIShaderTextEditorWindow.h"

UIShaderTextEditorWindow::UIShaderTextEditorWindow(Shader* _shader) : UIWindow(_shader->GetFragPath())
{
	auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	editor.SetLanguageDefinition(lang);
	shader = _shader;

	std::fstream fShaderFile;
	fShaderFile.exceptions(std::fstream::failbit | std::fstream::badbit);
	try
	{
		fShaderFile.open(shader->GetFragPath());
		std::stringstream fShaderStream;
		fShaderStream << fShaderFile.rdbuf();
		fShaderFile.close();
		text = fShaderStream.str();
	}
	catch (std::fstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	editor.SetText(text);
}

UIShaderTextEditorWindow::~UIShaderTextEditorWindow()
{
}

void UIShaderTextEditorWindow::WindowBody()
{
	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_Once);
	if (ImGui::Button("Compile shader"))
	{
		text = editor.GetText();
		std::ofstream fShaderFile;
		fShaderFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try
		{
			fShaderFile.open(shader->GetFragPath());
			fShaderFile << text;
			fShaderFile.close();
		}
		catch (std::ofstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_SAVED" << std::endl;
		}

		compilationError = !shader->Recompile(errorMsg);
	}

	auto cpos = editor.GetCursorPosition();

	if (compilationError)
	{
		ImGui::Text("\n\nCOMPILATION ERROR : \n%s\n\n", errorMsg.c_str());
	}

	ImGui::Text("%6d/%-6d %6d lines  | %s | %s\n", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
		editor.IsOverwrite() ? "Ovr" : "Ins",
		editor.CanUndo() ? "*" : " ");
	editor.Render("");
}
