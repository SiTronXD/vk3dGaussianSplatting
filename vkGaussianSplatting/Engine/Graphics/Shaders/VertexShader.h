#pragma once

#include "Shader.h"

class VertexShader : public Shader
{
private:
public:
	VertexShader();
	VertexShader(const Device& device, const std::string& filePath);
	~VertexShader();

	void createFromFile(const Device& device, const std::string& filePath);
};