#pragma once

#include <unordered_map>
#include <vector>
#include <happly.h>

#include "Graphics/Mesh.h"
#include "Graphics/Texture/Texture.h"
#include "Components.h"

struct GfxAllocContext;

class ResourceManager
{
private:
	std::unordered_map<std::string, uint32_t> nameToTexture;
	std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> nameToMesh; // { meshId, defaultMaterialId }

	std::vector<std::shared_ptr<Texture>> textures;
	std::vector<Mesh> meshes;
	std::vector<GaussianData> gaussians;

	const GfxAllocContext* gfxAllocContext;

	template <typename T>
	void loadPlyProperty(
		happly::Element& element, 
		const std::string& propertyStr, 
		std::vector<T>& output);

public:
	ResourceManager();
	~ResourceManager();

	void init(const GfxAllocContext& gfxAllocContext);
	void cleanup();

	void clearAllGaussians();

	uint32_t addMesh(
		const std::string& filePath, 
		Material& outputMeshMaterial,
		bool calculateNormals = true);
	uint32_t addTexture(const std::string& filePath);
	uint32_t addEmptyTexture();
	uint32_t addCubeMap(const std::vector<std::string>& filePaths);
	uint32_t addGaussian(const GaussianData& gaussianData);
	
	void loadGaussians(const std::string& filePath);

	inline Mesh& getMesh(uint32_t meshID) { return this->meshes[meshID]; }
	inline Texture* getTexture(uint32_t textureID) { return this->textures[textureID].get(); }
	inline const std::vector<GaussianData>& getGaussians() const { return this->gaussians; }

	inline size_t getNumMeshes() const { return this->meshes.size(); }
	inline size_t getNumTextures() const { return this->textures.size(); }
};

template<typename T>
inline void ResourceManager::loadPlyProperty(
	happly::Element& element,
	const std::string& propertyStr,
	std::vector<T>& output)
{
	assert(output.size() == 0);

	bool hasProperty = element.hasPropertyType<T>(propertyStr);
	assert(hasProperty);

	output = element.getProperty<T>(propertyStr);
}