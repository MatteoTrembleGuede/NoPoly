#include "UIResourceBrowser.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include "dirent.h"
#include <direct.h>
#include <fileapi.h>

std::string UIResourceBrowser::applicationPath;

void UIResourceBrowser::Init(std::string extensionsFilter, bool _forExporting)
{
	std::list<std::string> listExt;
	std::list<unsigned char> lastWord;
	char tmp[512];
	_getcwd(tmp, 512);
	currentPath = tmp;
	forExporting = _forExporting;

	for (int i = 0; i < extensionsFilter.size() + 1; ++i)
	{
		if (extensionsFilter[i] != '\0' && extensionsFilter[i] != '\n' && extensionsFilter[i] != ',')
		{
			lastWord.push_back(extensionsFilter[i]);
		}
		else
		{
			auto letter = lastWord.begin();
			int startOffset = 0;
			if (*letter == '.')
			{
				letter++;
				startOffset = 1;
			}
			char* ext = new char[lastWord.size() + 1 - startOffset];

			for (int j = startOffset; j < lastWord.size(); ++j)
			{
				ext[j - startOffset] = *letter;
				letter++;
			}
			ext[lastWord.size() - startOffset] = '\0';
			listExt.push_back(std::string(ext));
			delete[] ext;
			lastWord.clear();
		}
	}

	count = listExt.size();
	extensions = new std::string[count];
	auto nextExt = listExt.begin();

	for (int i = 0; i < count; ++i)
	{
		extensions[i] = *nextExt;
		nextExt++;
	}

	ChangeDirectory("");


	DWORD availableDrivesMask = GetLogicalDrives();

	char driveLetter = 'A';
	for (int i = 0; i < 26; ++i)
	{
		if (((availableDrivesMask >> i) & 1) == 1)
		{
			bool isValid;
			std::string driveSymbol = std::string(&driveLetter, 1) + ":\\";
			DIR* tmp = opendir(driveSymbol.c_str());

			isValid = (tmp && tmp->wdirp->cached != 0);

			if (isValid) availableDrives.push_back(driveSymbol);
		}
		driveLetter++;
	}

	int index = 0;
	for (std::string drive : availableDrives)
	{
		if (drive[0] == currentPath[0])
		{
			selectedDrive = index;
			break;
		}
		index++;
	}
}

UIResourceBrowser::~UIResourceBrowser()
{
	delete[] extensions;
}

bool UIResourceBrowser::HasRequestedExtension(std::string fileName, std::string extensions[], unsigned int count)
{
	for (int i = 0; i < count; ++i)
	{
		std::string currentExt = extensions[i];

		if (fileName.size() <= currentExt.size()) continue;

		for (int j = 0; j < currentExt.size(); ++j)
		{
			char charExt = currentExt[currentExt.size() - 1 - j];
			char charFil = fileName[fileName.size() - 1 - j];

			if (std::tolower(charExt) != std::tolower(charFil))
			{
				break;
			}

			if (j == currentExt.size() - 1 && (fileName.size() - 1 - (j + 1)) >= 0
				&& (fileName[fileName.size() - 1 - (j + 1)] == '.' || charFil == '.'))
			{
				return true;
			}
		}
	}

	return false;
}

std::string UIResourceBrowser::GetNoExtension(std::string fileName)
{
	for (int j = 0; j < fileName.size(); ++j)
	{
		char charFil = fileName[fileName.size() - 1 - j];

		if (charFil == '.')
		{
			return std::string(fileName.c_str(), fileName.size() - 1 - j);
		}
	}

	return fileName;
}

std::string UIResourceBrowser::GetExtension(std::string fileName)
{
	for (int j = 1; j < fileName.size(); ++j)
	{
		char charFil = fileName[fileName.size() - 1 - j];

		if (charFil == '.')
		{
			return std::string(fileName.c_str() + fileName.size() - j, j);
		}
	}

	return "no extension";
}

std::string RewindPath(std::string path)
{
	bool canGoBack = true;
	auto it = &path[path.size() - 2];
	while (*it != '\\' && *it != '/' && it > &path[0])
	{
		it -= 1;
	}

	if (it == &path[0]) canGoBack = false;

	if (canGoBack)
	{
		int size = it - &path[0] + 1;
		return std::string(path.c_str(), size);
	}

	return path;
}

std::string UIResourceBrowser::RemoveFileNameFromPath(std::string path)
{
	DIR* dir = opendir(path.c_str());

	if (dir && dir->wdirp->cached != 0)
	{
		closedir(dir);
		return path;
	}
	else
	{
		std::string rew =  RewindPath(path);
		if (rew == path) return "";
		else return rew;
	}
}

std::string UIResourceBrowser::GetFileNameFromPath(std::string path)
{
	std::string onlyPath = RemoveFileNameFromPath(path);
	if (onlyPath == path) return "";
	std::string name = std::string(path.c_str() + onlyPath.size(), path.size() - onlyPath.size());
	return name;
}

std::string UIResourceBrowser::GetRelativePath(std::string path, std::string referencePosition)
{
	std::string workingDir = referencePosition;
	std::string lastWD;
	std::string tmpPath;
	std::string lastPath;
	int rewindNum = 0;

	if (path[0] != workingDir[0]) return path;

	while (lastWD != workingDir)
	{
		tmpPath = path;
		while (lastPath != tmpPath)
		{
			if (workingDir == std::string(tmpPath.c_str(), tmpPath.size()))
			{
				path = path.c_str() + tmpPath.size();
				std::string rewindStack = "";
				for (int i = 0; i < rewindNum; ++i) rewindStack += "../";
				return rewindStack + path;
			}
			lastPath = tmpPath;
			tmpPath = RewindPath(tmpPath);
		}

		lastWD = workingDir;
		workingDir = RewindPath(workingDir);
		rewindNum++;
	}

	return path;
}

std::string UIResourceBrowser::GetWorkingDirectory()
{
	char tmp[512];
	_getcwd(tmp, 512);
	return tmp;
}

std::string UIResourceBrowser::GetApplicationDirectory()
{
	return applicationPath;
}

void UIResourceBrowser::SetApplicationDirectory(std::string _path)
{
	applicationPath = _path;
}

bool UIResourceBrowser::SetDirectory(std::string directoryPath)
{
	DIR* dir = opendir(directoryPath.c_str());

	if (dir && dir->wdirp->cached != 0)
	{
		closedir(dir);
		currentPath = directoryPath;
		inputPath = currentPath;
		GetFileListInDir(currentPath);
		selectedItem = -1;
		return true;
	}
	return false;
}

void UIResourceBrowser::ChangeDirectory(std::string directoryName)
{
	if (directoryName == "..")
	{
		currentPath = RewindPath(currentPath);
	}
	else
	{
		currentPath += directoryName + "\\";
	}
	inputPath = currentPath;
	GetFileListInDir(currentPath);
	selectedItem = -1;
}

void UIResourceBrowser::GetFileListInDir(std::string _path)
{
	DIR* directory = opendir(_path.c_str());
	struct dirent* currentEntry = nullptr;
	inDirList.clear();


	while (currentEntry = readdir(directory))
	{
		if (strcmp(currentEntry->d_name, ".") == 0 || strcmp(currentEntry->d_name, "..") == 0) continue;
		if (currentEntry->d_type == DT_REG)
		{
			if (HasRequestedExtension(currentEntry->d_name, extensions, count)) inDirList.push_back(InDirectoryItem(currentEntry->d_name, File));
		}
		else if (currentEntry->d_type == DT_DIR)
		{
			inDirList.push_back(InDirectoryItem(currentEntry->d_name, Directory));
		}
	}

	closedir(directory);
}

//bool GetItemInList(void* data, int idx, const char** out_text)
//{
//	std::list<InDirectoryItem>* inDirList = (std::list<InDirectoryItem>*)data;
//	if (idx >= 0 && idx < inDirList->size())
//	{
//		auto it = inDirList->begin();
//		for (int i = 0; i < idx; ++i) ++it;
//		*out_text = (*it).name.c_str();
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

//bool GetDriveInList(void* data, int idx, const char** out_text)
//{
//	std::list<std::string>* driveList = (std::list<std::string>*)data;
//	if (idx >= 0 && idx < driveList->size())
//	{
//		auto it = driveList->begin();
//		for (int i = 0; i < idx; ++i) ++it;
//		*out_text = (*it).c_str();
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

void UIResourceBrowser::DirectoryContent()
{
	int tmpSelectedItem = -1;
	//ImGui::ListBox("", &tmpSelectedItem, GetItemInList, (void*)&inDirList, inDirList.size(), 10);

	auto GetItemInList = [](void* data, int idx, const char** out_text)
	{
		std::list<InDirectoryItem>* inDirList = (std::list<InDirectoryItem>*)data;
		if (idx >= 0 && idx < inDirList->size())
		{
			auto it = inDirList->begin();
			for (int i = 0; i < idx; ++i) ++it;
			*out_text = (*it).name.c_str();
			return true;
		}
		else
		{
			return false;
		}
	};

	ImGui::ListBox("", &tmpSelectedItem, GetItemInList, (void*)&inDirList, inDirList.size(), 10);

	if (tmpSelectedItem > -1)
	{
		if (ImGui::GetTime() - lastClickTime > 0.25)
		{
			firstClickSelect = -1;
			secondClickSelect = -1;
			doubleClicked = false;
		}

		selectedItem = tmpSelectedItem;

		const char* name;

		if (GetItemInList((void*)&inDirList, selectedItem, &name))
		{
			inputFileName = name;
		}

		if (firstClickSelect < 0) firstClickSelect = selectedItem;
		else secondClickSelect = selectedItem;

		lastClickTime = ImGui::GetTime();
	}

	if (ImGui::IsMouseDoubleClicked(0)) doubleClicked = true;

	if (doubleClicked && firstClickSelect > -1 && secondClickSelect > -1)
	{
		if (firstClickSelect == secondClickSelect)
		{
			auto it = inDirList.begin();
			for (int i = 0; i < selectedItem; ++i) ++it;
			if ((*it).type == Directory) ChangeDirectory((*it).name);
			else if ((*it).type == File) ExecuteCommand();
		}

		doubleClicked = false;
		firstClickSelect = secondClickSelect = -1;
	}
}

void UIResourceBrowser::InputDirectory()
{
	const char* tmpPath = inputPath.c_str();
	char* newPath = new char[128];
	strcpy(newPath, tmpPath);
	ImGui::InputText("Path", newPath, 128);
	inputPath = newPath;
	delete[] newPath;

	ImGui::SameLine();

	if (ImGui::Button("Go"))
	{
		SetDirectory(inputPath);
	}
	ImGui::SameLine();

	if (ImGui::Button("Back")) ChangeDirectory("..");
}

void UIResourceBrowser::SelectDrive()
{
	int oldSelected = selectedDrive;
	//ImGui::Combo("Select drive", &selectedDrive, GetDriveInList, &availableDrives, availableDrives.size());
	ImGui::Combo("Select drive", &selectedDrive,
		[](void* data, int idx, const char** out_text)
		{
			std::list<std::string>* driveList = (std::list<std::string>*)data;
			if (idx >= 0 && idx < driveList->size())
			{
				auto it = driveList->begin();
				for (int i = 0; i < idx; ++i) ++it;
				*out_text = (*it).c_str();
				return true;
			}
			else
			{
				return false;
			}
		}
	, &availableDrives, availableDrives.size());

	if (oldSelected != selectedDrive)
	{
		auto it = availableDrives.begin();
		for (int i = 0; i < selectedDrive; ++i) ++it;
		SetDirectory((*it));
	}
}

void UIResourceBrowser::InputFileName()
{
	const char* tmpFileName = inputFileName.c_str();
	char* newFileName = new char[128];
	strcpy(newFileName, tmpFileName);
	ImGui::InputText("Name", newFileName, 128);
	inputFileName = newFileName;
	delete[] newFileName;

	if (forExporting)
	{
		ImGui::Combo("Extension", &selectedExtension,
			[](void* data, int idx, const char** out_text)
			{
				std::string* extList = (std::string*)data;
				if (idx >= 0)
				{
					*out_text = extList[idx].c_str();
					return true;
				}
				else
				{
					return false;
				}
			}, extensions, count);
	}
}

void UIResourceBrowser::AlertNoSuchFile()
{
	ImGui::OpenPopup("alert");
	ImGui::BeginPopup("alert");

	ImGui::Text(("No file exists with this name : " + inputFileName).c_str());

	if (ImGui::Button("ok"))
	{
		alertNoSuchFile = false;
	}

	ImGui::EndPopup();
}

void UIResourceBrowser::ConfirmAlert()
{
	ImGui::OpenPopup("alert");
	ImGui::BeginPopup("alert");

	ImGui::Text(("A file already exists with this name : " + inputFileName + "\nWould you like to erase it ?").c_str());

	if (ImGui::Button("yes"))
	{
		forExporting = false;
	}

	if (ImGui::Button("no"))
	{
		alertAlreadyExistingFile = false;
	}

	ImGui::EndPopup();
}

bool UIResourceBrowser::CheckFileExists()
{
	std::string testName;
	std::ifstream testFile;

	selectedExtension = selectedExtension < -1 ? 0 : selectedExtension;

	if (forExporting)
	{
		testName = currentPath + GetNoExtension(inputFileName) + (extensions[selectedExtension][0] == '.' ? "" : ".") + extensions[selectedExtension];
	}
	else
	{
		testName = currentPath + inputFileName;
	}

	testFile.open(testName, std::ios_base::_Nocreate);

	if (testFile.is_open())
	{
		testFile.close();
		return true;
	}
	else
	{
		return false;
	}
}

void UIResourceBrowser::ExecuteCommand()
{
	if (forExporting)
	{
		if (selectedExtension < 0) return;

		if (!alertAlreadyExistingFile)
		{
			if (CheckFileExists())
			{
				alertAlreadyExistingFile = true;
			}
		}

		if (alertAlreadyExistingFile)
		{
			ConfirmAlert();
			return;
		}
	}
	else
	{
		if (!alertNoSuchFile)
		{
			if (!CheckFileExists())
			{
				alertNoSuchFile = true;
			}
		}
		if (alertNoSuchFile)
		{
			AlertNoSuchFile();
			return;
		}
	}

	if (forExporting)
	{
		inputFileName = UIResourceBrowser::GetNoExtension(inputFileName) + "." + extensions[selectedExtension];
	}

	command(currentPath, inputFileName, commandData);
	Destroy();
}

void UIResourceBrowser::CommandButton()
{
	if (ImGui::Button("Confirm") || alertAlreadyExistingFile || alertNoSuchFile)
	{
		ExecuteCommand();
	}

	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		Destroy();
	}
}

void UIResourceBrowser::WindowBody()
{
	InputDirectory();
	SelectDrive();
	DirectoryContent();
	InputFileName();
	CommandButton();
}