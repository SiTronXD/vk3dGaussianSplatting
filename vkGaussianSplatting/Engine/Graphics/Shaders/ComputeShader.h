#pragma once

#include "Shader.h"

class ComputeShader : public Shader
{
private:
public:
	ComputeShader();
	ComputeShader(
		const Device& device, 
		const std::string& filePath,
		const std::vector<SpecializationConstant>& specializationConstants);
	~ComputeShader();

	void createFromFile(
		const Device& device, 
		const std::string& filePath,
		const std::vector<SpecializationConstant>& specializationConstants);
};