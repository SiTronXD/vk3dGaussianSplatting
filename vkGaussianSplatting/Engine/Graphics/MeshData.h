#pragma once

#include <string>
#include "Buffer/VertexBuffer.h"

struct Submesh
{
	uint32_t startIndex;
	uint32_t numIndices;
};

class MeshData
{
private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Submesh> submeshes;

public:
	MeshData();
	~MeshData();

	void create(
		std::vector<Vertex>& vertices, 
		std::vector<uint32_t>& indices);

	bool loadOBJ(const std::string& filePath, bool calculateNormals = true);

	inline std::vector<Vertex>& getVertices() { return this->vertices; }
	inline std::vector<uint32_t>& getIndices() { return this->indices; }
	inline std::vector<Submesh>& getSubmeshes() { return this->submeshes; }
};