#include "pch.h"
#include "MaterialSet.h"

void MaterialSet::createMaterialSet(const Mesh& mesh)
{
	this->materials.resize(mesh.getSubmeshes().size(), SubmeshMaterial());
}

void MaterialSet::applySubmeshMaterial(
	uint32_t submeshMaterialIndex, 
	const SubmeshMaterial& newSubmeshMaterial)
{
	this->materials[submeshMaterialIndex] = newSubmeshMaterial;
}

void MaterialSet::applyUniformSubmeshMaterial(const SubmeshMaterial& newSubmeshMaterial)
{
	// Copy new submesh material into all submesh materials
	for (size_t i = 0; i < this->materials.size(); ++i)
	{
		this->applySubmeshMaterial(i, newSubmeshMaterial);
	}
}
