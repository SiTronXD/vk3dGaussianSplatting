#include "pch.h"
#include "Mesh.h"

Mesh::Mesh()
	: numIndices(0)
{
}

Mesh::~Mesh()
{
}

void Mesh::createMesh(
	const GfxAllocContext& gfxAllocContext,
	MeshData& meshData)
{
	this->vertexBuffer.createVertexBuffer(gfxAllocContext, meshData.getVertices());
	this->indexBuffer.createIndexBuffer(gfxAllocContext, meshData.getIndices());

	this->submeshes = std::vector<Submesh>(meshData.getSubmeshes());

	this->numIndices = uint32_t(meshData.getIndices().size());
}

void Mesh::cleanup()
{
	this->indexBuffer.cleanup();
	this->vertexBuffer.cleanup();
}
