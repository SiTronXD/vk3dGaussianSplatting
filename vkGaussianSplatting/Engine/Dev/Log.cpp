#include "pch.h"

#include <comdef.h>

#include "Log.h"

void Log::popupWindow(const std::string& title, const std::string& message)
{
	// Convert const char* to LPCWSTR
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, wString, 4096);

	// Simple message box
	MessageBox(
		NULL, wString, std::wstring(title.begin(), title.end()).c_str(), MB_OK
	);

	delete[] wString;
}

void Log::write(const std::string& message)
{
	std::cout << "[Log]: " << message << std::endl;
}

void Log::writeAlert(const std::string& message)
{
	std::cout << "----------- [Log] -----------" << std::endl;
	std::cout << message << std::endl;
	std::cout << "----------- [Log End] -----------" << std::endl;

	Log::alert(message);
}

void Log::warning(const std::string& message)
{
	std::cout << "[Log Warning]: " << message << std::endl;
}

void Log::error(const std::string& errorMessage)
{
	Log::popupWindow("ERROR", errorMessage);
}

void Log::alert(const std::string& message)
{
	Log::popupWindow("ALERT", message);
}