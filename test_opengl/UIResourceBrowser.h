#pragma once

#ifndef UIRESOURCE_BROWSER_H
#define UIRESOURCE_BROWSER_H

#include "UIWindow.h"
#include "Delegate.h"

enum InDirectoryType
{
	File,
	Directory
};

typedef struct InDirectoryItem
{
	std::string name;
	InDirectoryType type;
	InDirectoryItem(std::string _name, InDirectoryType _type) : name(_name), type(_type) {}
}InDirectoryItem;


class UIResourceBrowser:
	public UIWindow
{
public:

	DeclareDelegate(ResourceBrowserCMD, std::string, std::string, void*);
	ResourceBrowserCMD command;

	template<class T>
	UIResourceBrowser(std::string _name, std::string extensions, T* commandOwnerPtr, void (T::*cmd)(std::string, std::string, void*), void* cmdData = nullptr, bool _forExporting = false);
	~UIResourceBrowser();

	static bool HasRequestedExtension(std::string fileName, std::string extensions[], unsigned int count);
	static std::string GetNoExtension(std::string fileName);
	static std::string GetExtension(std::string fileName);
	static std::string RemoveFileNameFromPath(std::string path);
	static std::string GetFileNameFromPath(std::string path);
	static std::string GetRelativePath(std::string path, std::string referencePosition);
	static std::string GetWorkingDirectory();
	static std::string GetApplicationDirectory();
	static void SetApplicationDirectory(std::string _path);

protected:

	static std::string applicationPath;

	// AVAILABLE DRIVES
	int selectedDrive = -1;
	std::list<std::string> availableDrives;

	// CURRENT LOCATION
	std::string currentPath;
	std::string inputPath;
	std::list<InDirectoryItem> inDirList;

	// TARGET DATA
	std::string inputFileName;
	void* commandData;

	// ALERT
	bool forExporting;
	bool alertAlreadyExistingFile = false;
	bool alertNoSuchFile = false;

	// CLICK NAVIGATION
	int selectedItem = -1;
	int firstClickSelect = -1;
	int secondClickSelect = -1;
	bool doubleClicked = false;
	double lastClickTime = 0;

	// INITIALISATION
	void Init(std::string extensionsFilter, bool _forExporting);

	// EXTENSION FILTER
	std::string* extensions;
	unsigned int count;
	int selectedExtension = 0;

	// DIRECTORY NAVIGATION
	bool SetDirectory(std::string directoryPath);
	void ChangeDirectory(std::string directoryName);
	void GetFileListInDir(std::string _path);

	// WINDOW BODY DESCRIPTION
	void DirectoryContent();
	void InputDirectory();
	void SelectDrive();
	void InputFileName();
	void CommandButton();
	virtual void WindowBody() override;

	// COMMAND EXECUTION
	void AlertNoSuchFile();
	void ConfirmAlert();
	bool CheckFileExists();
	void ExecuteCommand();
};

template<class T>
UIResourceBrowser::UIResourceBrowser(std::string _name, std::string extensionsFilter, T* commandOwnerPtr, void (T::* cmd)(std::string, std::string, void*), void* cmdData, bool _forExporting) : UIWindow(_name)
{
	commandData = cmdData;
	command.Bind(commandOwnerPtr, cmd);
	Init(extensionsFilter, _forExporting);
}

#endif