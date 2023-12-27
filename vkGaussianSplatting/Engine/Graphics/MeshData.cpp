#include "pch.h"

#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj.h>

#include "MeshData.h"

MeshData::MeshData()
{
}

MeshData::~MeshData()
{
}

void MeshData::create(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	this->vertices.reserve(vertices.size());
	this->indices.reserve(indices.size());

	this->vertices.assign(vertices.begin(), vertices.end());
	this->indices.assign(indices.begin(), indices.end());
}

bool MeshData::loadOBJ(const std::string& filePath, bool calculateNormals)
{
	// Load mesh
	fastObjMesh* loadedObj = fast_obj_read(filePath.c_str());
	if (!loadedObj)
	{
		Log::error("Failed to load mesh: " + filePath);
		return false;
	}

	// Make sure entire mesh is triangulated
	for (unsigned int i = 0; i < loadedObj->face_count; ++i)
	{
		if (loadedObj->face_vertices[i] != 3)
		{
			Log::error(filePath + " is not triangulated.");
			return false;
		}
	}

	// Positions
	this->vertices.resize(loadedObj->index_count);
	for (unsigned int i = 0; i < loadedObj->position_count; ++i)
	{
		Vertex& v = this->vertices[i];
		v.pos.x = loadedObj->positions[i * 3 + 0];
		v.pos.y = loadedObj->positions[i * 3 + 1];
		v.pos.z = loadedObj->positions[i * 3 + 2];
	}

	// Indices
	this->indices.resize(loadedObj->index_count);
	for (unsigned int i = 0; i < loadedObj->index_count; ++i)
	{
		this->indices[i] = loadedObj->indices[i].p;

		Vertex& v = this->vertices[this->indices[i]];

		// Load texcoords
		v.texCoord.x = loadedObj->texcoords[loadedObj->indices[i].t * 2 + 0];
		v.texCoord.y = loadedObj->texcoords[loadedObj->indices[i].t * 2 + 1];
	}

	// Submeshes
	this->submeshes.resize(loadedObj->object_count);
	for (size_t i = 0; i < this->submeshes.size(); ++i)
	{
		const fastObjGroup& currentObject = loadedObj->objects[i];

		this->submeshes[i].startIndex = currentObject.index_offset;
		this->submeshes[i].numIndices = currentObject.face_count * 3;
	}

	if (!calculateNormals)
	{
		// Load normals
		for (unsigned int i = 0; i < loadedObj->index_count; ++i)
		{
			Vertex& v = this->vertices[this->indices[i]];
			v.normal.x = loadedObj->normals[loadedObj->indices[i].n * 3 + 0];
			v.normal.y = loadedObj->normals[loadedObj->indices[i].n * 3 + 1];
			v.normal.z = loadedObj->normals[loadedObj->indices[i].n * 3 + 2];
		}
	}
	else
	{
		// Calculate normals from triangles
		for (size_t i = 0; i < this->indices.size(); i += 3)
		{
			Vertex& v0 = this->vertices[this->indices[i + 0]];
			Vertex& v1 = this->vertices[this->indices[i + 1]];
			Vertex& v2 = this->vertices[this->indices[i + 2]];

			const glm::vec3 edge0 = v1.pos - v0.pos;
			const glm::vec3 edge1 = v2.pos - v0.pos;

			glm::vec3 normal = glm::cross(edge0, edge1);
			normal = glm::normalize(normal);

			v0.normal += normal;
			v1.normal += normal;
			v2.normal += normal;
		}
	}

	// Normalize smooth normals
	for (size_t i = 0; i < this->vertices.size(); ++i)
	{
		Vertex& v = this->vertices[i];
		if (glm::dot(v.normal, v.normal) > 0.0f)
			v.normal = glm::normalize(v.normal);
	}

	// Destroy loaded obj model
	fast_obj_destroy(loadedObj);

	return true;
}
