#include "pch.h"
#include "ResourceManager.h"
#include "Graphics/Mesh.h"
#include "Graphics/MeshData.h"
#include "Graphics/Texture/TextureCube.h"
#include "Graphics/Texture/Texture2D.h"
#include "../Dev/StrHelper.h"

ResourceManager::ResourceManager()
	: gfxAllocContext(nullptr)
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

void ResourceManager::clearAllGaussians()
{
	this->gaussians.clear();
	this->gaussians.shrink_to_fit();
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
	uint32_t defaultMaterialIndex = 0;
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

uint32_t ResourceManager::addGaussian(const GaussianData& gaussianData)
{
	uint32_t gaussianId = (uint32_t) this->gaussians.size();

	this->gaussians.push_back(gaussianData);

	return gaussianId;
}

void ResourceManager::loadGaussians(const std::string& filePath)
{
	if (!std::filesystem::exists(filePath))
	{
		Log::error("File cannot be found: " + filePath);
		return;
	}

	// Load ply file
	happly::PLYData plyData(filePath);

	const std::vector<std::string> names = plyData.getElementNames();
	happly::Element& element = plyData.getElement(names[0]);
	const std::vector<std::string> propertyNames = element.getPropertyNames();
	const size_t count = element.count;

	std::vector<float> gPositionsX;
	std::vector<float> gPositionsY;
	std::vector<float> gPositionsZ;
	this->loadPlyProperty<float>(element, "x", gPositionsX);
	this->loadPlyProperty<float>(element, "y", gPositionsY);
	this->loadPlyProperty<float>(element, "z", gPositionsZ);

	std::vector<float> gScalesX;
	std::vector<float> gScalesY;
	std::vector<float> gScalesZ;
	this->loadPlyProperty<float>(element, "scale_0", gScalesX);
	this->loadPlyProperty<float>(element, "scale_1", gScalesY);
	this->loadPlyProperty<float>(element, "scale_2", gScalesZ);

	std::vector<float> gRot0;
	std::vector<float> gRot1;
	std::vector<float> gRot2;
	std::vector<float> gRot3;
	this->loadPlyProperty<float>(element, "rot_0", gRot0);
	this->loadPlyProperty<float>(element, "rot_1", gRot1);
	this->loadPlyProperty<float>(element, "rot_2", gRot2);
	this->loadPlyProperty<float>(element, "rot_3", gRot3);

	std::vector<float> gOpacities;
	this->loadPlyProperty<float>(element, "opacity", gOpacities);

	std::vector<float> gRedSh00;
	std::vector<float> gGreenSh00;
	std::vector<float> gBlueSh00;
	this->loadPlyProperty<float>(element, "f_dc_0", gRedSh00);
	this->loadPlyProperty<float>(element, "f_dc_1", gGreenSh00);
	this->loadPlyProperty<float>(element, "f_dc_2", gBlueSh00);

	uint32_t numRestCoeffs = 15;
	std::vector<std::vector<float>> gShRest(numRestCoeffs * 3);
	for (size_t i = 0; i < gShRest.size(); ++i)
	{
		this->loadPlyProperty<float>(element, "f_rest_" + std::to_string(i), gShRest[i]);
	}

	uint32_t numGaussians = (uint32_t)gPositionsX.size();
	this->gaussians.resize(numGaussians);
	glm::vec3 minPos(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	glm::vec3 maxPos(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	for (uint32_t i = 0; i < numGaussians; ++i)
	{
		GaussianData& gaussian = this->gaussians[i];

		gaussian.position = glm::vec4(
			gPositionsX[i] * -1.0f,
			gPositionsY[i] * -1.0f,
			gPositionsZ[i],
			0.0f
		);
		gaussian.scale = glm::vec4(
			std::exp(gScalesX[i]),
			std::exp(gScalesY[i]),
			std::exp(gScalesZ[i]),
			0.0f
		);
		{
			gaussian.rot = glm::vec4(
				gRot0[i],
				gRot1[i],
				gRot2[i],
				gRot3[i]
			);
			gaussian.rot = glm::normalize(gaussian.rot);
			gaussian.rot = glm::vec4(
				-gaussian.rot[2],
				-gaussian.rot[3],
				gaussian.rot[0],
				-gaussian.rot[1]
			);
		}

		gaussian.shCoeffs[0] = glm::vec4(
			gRedSh00[i],
			gGreenSh00[i],
			gBlueSh00[i],
			(1.0f / (1.0f + std::exp(-gOpacities[i])))
		);
		for (uint32_t c = 0; c < numRestCoeffs; ++c)
		{
			gaussian.shCoeffs[c + 1] = glm::vec4(
				gShRest[c + numRestCoeffs * 0][i],
				gShRest[c + numRestCoeffs * 1][i],
				gShRest[c + numRestCoeffs * 2][i],
				0.0f
			);
		}

		maxPos.x = std::max(maxPos.x, gaussian.position.x);
		maxPos.y = std::max(maxPos.y, gaussian.position.y);
		maxPos.z = std::max(maxPos.z, gaussian.position.z);

		minPos.x = std::min(minPos.x, gaussian.position.x);
		minPos.y = std::min(minPos.y, gaussian.position.y);
		minPos.z = std::min(minPos.z, gaussian.position.z);
	}
	const glm::vec3 deltaMinMax = maxPos - minPos;
	const uint32_t mortonSpaceSize = (1 << 10) - 1;

	// Sort gaussians to be more cache coherent
	std::sort(
		std::begin(this->gaussians), 
		std::end(this->gaussians), 
		[&](const GaussianData& a, const GaussianData& b) 
		{
			const glm::vec3 mortonSpacePosA = (glm::vec3(a.position) - minPos) / deltaMinMax * (float) mortonSpaceSize;
			const glm::vec3 mortonSpacePosB = (glm::vec3(b.position) - minPos) / deltaMinMax * (float) mortonSpaceSize;

			return SMath::encodeZorderCurve(glm::uvec3(mortonSpacePosA)) < SMath::encodeZorderCurve(glm::uvec3(mortonSpacePosB));
		}
	);

	Log::write("Number of gaussians: " + std::to_string(this->gaussians.size()));
}
