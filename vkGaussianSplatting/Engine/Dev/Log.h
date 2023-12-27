#pragma once

#include <string>

class Log
{
private:
	static void popupWindow(const std::string& title, const std::string& message);

public:
	static void write(const std::string& message);
	static void writeAlert(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& errorMessage);
	static void alert(const std::string& errorMessage);
};
