#include "pch.h"
#include "ResourceManager.h"
#include "Graphics/Mesh.h"
#include "Graphics/MeshData.h"
#include "Graphics/Texture/TextureCube.h"
#include "Graphics/Texture/Texture2D.h"

ResourceManager::ResourceManager()
	: gfxAllocContext(nullptr),
	numCoefficientsPerAngle(0),
	numCoefficientsCosTermPerAngle(0)
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::init(const GfxAllocContext& gfxAllocContext)
{
	this->gfxAllocContext = &gfxAllocContext;
}

void ResourceManager::cleanup()
{
	// Meshes
	for (size_t i = 0; i < this->meshes.size(); ++i)
	{
		this->meshes[i].cleanup();
	}
	this->meshes.clear();

	// Textures
	for (size_t i = 0; i < this->textures.size(); ++i)
	{
		this->textures[i]->cleanup();
	}
	this->textures.clear();
	this->textures.shrink_to_fit();
}

uint32_t ResourceManager::addMesh(
	const std::string& filePath, 
	Material& outputMeshMaterial,
	bool calculateNormals)
{
	// Check if mesh has already been added
	if (this->nameToMesh.count(filePath) != 0)
	{
		outputMeshMaterial.materialSetIndex = this->nameToMesh[filePath].second;

		return this->nameToMesh[filePath].first;
	}

	// Mesh index
	uint32_t createdMeshIndex = uint32_t(this->meshes.size());

	// Mesh
	this->meshes.push_back(Mesh());
	Mesh& createdMesh = this->meshes[createdMeshIndex];

	// Load mesh data
	MeshData meshData;
	meshData.loadOBJ(filePath, calculateNormals);

	// Create mesh from mesh data
	createdMesh.createMesh(
		*this->gfxAllocContext, 
		meshData
	);

	// Material
	uint32_t defaultMaterialIndex = this->addMaterial(createdMesh);
	outputMeshMaterial.materialSetIndex = defaultMaterialIndex;

	this->nameToMesh.insert({ filePath, { createdMeshIndex, defaultMaterialIndex } });

	return createdMeshIndex;
}

uint32_t ResourceManager::addTexture(const std::string& filePath)
{
	// Check if texture has already been added
	if (this->nameToTexture.count(filePath) != 0)
	{
		return this->nameToTexture[filePath];
	}

	// Texture resource
	uint32_t createdTextureIndex = this->addEmptyTexture();
	Texture2D* createdTexture = static_cast<Texture2D*>(this->textures[createdTextureIndex].get());
	this->nameToTexture.insert({ filePath, createdTextureIndex });

	// Load texture
	createdTexture->createFromFile(
		*this->gfxAllocContext, 
		filePath
	);

	return createdTextureIndex;
}

uint32_t ResourceManager::addEmptyTexture()
{
	uint32_t createdTextureIndex = uint32_t(this->textures.size());

	// Add texture
	this->textures.push_back(std::shared_ptr<Texture2D>(new Texture2D()));

	return createdTextureIndex;
}

uint32_t ResourceManager::addCubeMap(const std::vector<std::string>& filePaths)
{
	// Check if texture has already been added
	if (this->nameToTexture.count(filePaths[0]) != 0)
	{
		return this->nameToTexture[filePaths[0]];
	}

	// HDR for one single file
	if (filePaths.size() == 1)
	{
		if (filePaths[0].substr(filePaths[0].size() - 4, 4) != ".hdr")
		{
			Log::error("Unsupported float texture for cube map. Expected .hdr");
			return ~0u;
		}
	}
	// Non-HDR for 6 files
	else if (filePaths.size() != 6)
	{
		Log::error("The number of file paths is not 6 when adding cube map resource.");
		return ~0u;
	}

	uint32_t createdTextureIndex = uint32_t(this->textures.size());
	this->nameToTexture.insert({ filePaths[0], createdTextureIndex });

	// Load and create cube map resource
	std::shared_ptr<TextureCube> createdCubeMap(new TextureCube());
	createdCubeMap->createCubeMapFromFile(
		*this->gfxAllocContext,
		filePaths
	);

	// Add resources to list
	this->textures.push_back(createdCubeMap);

	return createdTextureIndex;
}

uint32_t ResourceManager::addBRDF(const std::string& filePath)
{
	// Check if material has already been added
	if (this->nameToBrdf.count(filePath) != 0)
	{
		return this->nameToBrdf[filePath];
	}

	// Add BRDF name
	uint32_t createdBrdfIndex = uint32_t(this->brdfs.size());
	this->nameToBrdf.insert({ filePath, createdBrdfIndex });

	// Create BRDF resource
	this->brdfs.push_back(BRDFData());
	BRDFData& createdBrdf = this->brdfs[createdBrdfIndex];

	// Load BRDF
	createdBrdf.createFromFile(filePath);

	// Create references for validation
	if (createdBrdfIndex == 0)
	{
		this->numCoefficientsPerAngle = uint32_t(createdBrdf.getShCoefficientSets()[0].size());
		this->numCoefficientsCosTermPerAngle = uint32_t(createdBrdf.getShCoefficientCosSets()[0].size());
	}

	// Validate number of angles
	if (ResourceManager::NUM_ANGLES != uint32_t(createdBrdf.getShCoefficientSets().size()))
	{
		Log::error("The number of angles (" + std::to_string(ResourceManager::NUM_ANGLES) + ") does not match between all BRDF files.");
		return 0;
	}
	if (ResourceManager::NUM_ANGLES != uint32_t(createdBrdf.getShCoefficientCosSets().size()))
	{
		Log::error("The number of angles (" + std::to_string(ResourceManager::NUM_ANGLES) + ") does not match between all BRDF files.");
		return 0;
	}

	// Validate number of coefficients within each SH set per angle
	for (uint32_t i = 0; i < ResourceManager::NUM_ANGLES; ++i)
	{
		// No cos term
		if (this->numCoefficientsPerAngle != uint32_t(createdBrdf.getShCoefficientSets()[i].size()))
		{
			Log::error("The number of SH coefficients per set (" + std::to_string(this->numCoefficientsPerAngle) + ") does not match between all SH sets.");
			return 0;
		}

		// Cos term
		if (this->numCoefficientsCosTermPerAngle != uint32_t(createdBrdf.getShCoefficientCosSets()[i].size()))
		{
			Log::error("The number of SH coefficients with cos term per set (" + std::to_string(this->numCoefficientsCosTermPerAngle) + ") does not match between all SH sets.");
			return 0;
		}
	}

	return createdBrdfIndex;
}

uint32_t ResourceManager::addMaterial(const Mesh& mesh)
{
	uint32_t materialIndex = uint32_t(this->materialSets.size());

	this->materialSets.push_back(MaterialSet());
	this->materialSets[materialIndex].createMaterialSet(mesh);

	return materialIndex;
}
