#pragma once

#include "Shader.h"

class FragmentShader : public Shader
{
private:
public:
	FragmentShader();
	FragmentShader(const Device& device, const std::string& filePath, const std::vector<SpecializationConstant>& specializationConstants);
	~FragmentShader();

	void createFromFile(const Device& device, const std::string& filePath, const std::vector<SpecializationConstant>& specializationConstants);
};