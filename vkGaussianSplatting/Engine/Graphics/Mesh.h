#pragma once

#include "Buffer/VertexBuffer.h"
#include "Buffer/IndexBuffer.h"
#include "MeshData.h"

struct GfxAllocContext;

class Mesh
{
private:
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;

	std::vector<Submesh> submeshes;

	uint32_t numIndices;

public:
	Mesh();
	~Mesh();

	void createMesh(
		const GfxAllocContext& gfxAllocContext,
		MeshData& meshData);

	void cleanup();

	inline const VertexBuffer& getVertexBuffer() const { return this->vertexBuffer; }
	inline const IndexBuffer& getIndexBuffer() const { return this->indexBuffer; }

	inline const std::vector<Submesh>& getSubmeshes() const { return this->submeshes; }

	inline uint32_t getNumIndices() const { return this->numIndices; }
};