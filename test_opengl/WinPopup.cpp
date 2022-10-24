#include "WinPopup.h"
#include <windows.h>

WinPopup::WinPopup(const char* message, const char* title, MessageType type)
{
	switch (type)
	{
	case Info:
		MessageBoxA(nullptr, message, title, MB_OK);
		break;
	case Warning:
		MessageBoxA(nullptr, message, title, MB_ICONWARNING | MB_OK);
		break;
	case Error:
		MessageBoxA(nullptr, message, title, MB_ICONERROR | MB_OK);
		break;
	}
}
