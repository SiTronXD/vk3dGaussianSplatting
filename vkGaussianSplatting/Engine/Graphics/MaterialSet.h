#pragma once

#include "Mesh.h"

struct SubmeshMaterial
{
	uint32_t brdfIndex = 0; // Default to avoid issues within shader
};

class MaterialSet
{
private:
	std::vector<SubmeshMaterial> materials;

public:
	void createMaterialSet(const Mesh& mesh);

	void applySubmeshMaterial(uint32_t submeshMaterialIndex, const SubmeshMaterial& newSubmeshMaterial);
	void applyUniformSubmeshMaterial(const SubmeshMaterial& newSubmeshMaterial);

	const std::vector<SubmeshMaterial>& getMaterialSet() const { return this->materials; }
};