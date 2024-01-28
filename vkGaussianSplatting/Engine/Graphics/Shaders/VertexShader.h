#pragma once

#include "Shader.h"

class VertexShader : public Shader
{
private:
public:
	VertexShader();
	VertexShader(const Device& device, const std::string& filePath, const std::vector<SpecializationConstant>& specializationConstants);
	~VertexShader();

	void createFromFile(
		const Device& device, 
		const std::string& filePath, 
		const std::vector<SpecializationConstant>& specializationConstants);
};