#pragma once

#include <unordered_map>
#include <vector>
#include "Graphics/MaterialSet.h"
#include "Graphics/Mesh.h"
#include "Graphics/BRDFData.h"
#include "Graphics/Texture/Texture.h"
#include "Components.h"

struct GfxAllocContext;

class ResourceManager
{
private:
	// Used to validate BRDF sets
	const uint32_t NUM_ANGLES = 90;
	uint32_t numCoefficientsPerAngle;
	uint32_t numCoefficientsCosTermPerAngle;

	std::unordered_map<std::string, uint32_t> nameToTexture;
	std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> nameToMesh; // { meshId, defaultMaterialId }
	std::unordered_map<std::string, uint32_t> nameToBrdf;

	std::vector<std::shared_ptr<Texture>> textures;
	std::vector<Mesh> meshes;
	std::vector<BRDFData> brdfs;
	std::vector<MaterialSet> materialSets;

	const GfxAllocContext* gfxAllocContext;

public:
	ResourceManager();
	~ResourceManager();

	void init(const GfxAllocContext& gfxAllocContext);
	void cleanup();

	uint32_t addMesh(
		const std::string& filePath, 
		Material& outputMeshMaterial,
		bool calculateNormals = true);
	uint32_t addTexture(const std::string& filePath);
	uint32_t addEmptyTexture();
	uint32_t addCubeMap(const std::vector<std::string>& filePaths);
	uint32_t addBRDF(const std::string& filePath);
	uint32_t addMaterial(const Mesh& mesh);

	inline Mesh& getMesh(uint32_t meshID) { return this->meshes[meshID]; }
	inline Texture* getTexture(uint32_t textureID) { return this->textures[textureID].get(); }
	inline BRDFData& getBRDFData(uint32_t brdfID) { return this->brdfs[brdfID]; }
	inline MaterialSet& getMaterialSet(uint32_t materialSetID) { return this->materialSets[materialSetID]; }

	inline size_t getNumMeshes() const { return this->meshes.size(); }
	inline size_t getNumTextures() const { return this->textures.size(); }
	inline size_t getNumBRDFs() const { return this->brdfs.size(); }
};