#pragma once

enum MessageType
{
	Info,
	Warning,
	Error
};

class WinPopup
{
public:

	WinPopup(const char* message, const char* title, MessageType type);

};

