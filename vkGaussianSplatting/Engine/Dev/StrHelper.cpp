#include "pch.h"
#include "StrHelper.h"

void StrHelper::splitString(
	std::string& str, 
	char delim, 
	std::vector<std::string>& output)
{
	size_t pos = 0;
	while ((pos = str.find(delim)) != std::string::npos)
	{
		// Extract and add token
		output.emplace_back(str.substr(0, pos));

		// Remove front
		str.erase(0, pos + 1);
	}

	// Add remaining string
	output.emplace_back(str);
}

std::string StrHelper::toTimingStr(float x)
{
	// Round value
	float roundedX = SMath::roundToThreeDecimals(x);
	std::string s = std::to_string(roundedX);

	// Split string
	std::vector<std::string> splitStrings; 
	StrHelper::splitString(s, '.', splitStrings);

	// Keep 3 chars
	splitStrings[1] = splitStrings[1].substr(0, 3);

	return splitStrings[0] + "." + splitStrings[1];
}

std::string StrHelper::vecToStr(const glm::vec3& vec)
{
	return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
}
