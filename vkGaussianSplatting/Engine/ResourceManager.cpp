#include "pch.h"
#include "ResourceManager.h"
#include "Graphics/Mesh.h"
#include "Graphics/MeshData.h"
#include "Graphics/Texture/TextureCube.h"
#include "Graphics/Texture/Texture2D.h"

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

	std::vector<float> gFeature0;
	std::vector<float> gFeature1;
	std::vector<float> gFeature2;

	this->loadPlyProperty<float>(element, "f_dc_0", gFeature0);
	this->loadPlyProperty<float>(element, "f_dc_1", gFeature1);
	this->loadPlyProperty<float>(element, "f_dc_2", gFeature2);

	std::vector<float> gFeatureRest0;
	std::vector<float> gFeatureRest1;
	std::vector<float> gFeatureRest2;

	this->loadPlyProperty<float>(element, "f_rest_0", gFeatureRest0);
	this->loadPlyProperty<float>(element, "f_rest_1", gFeatureRest1);
	this->loadPlyProperty<float>(element, "f_rest_2", gFeatureRest2);

	// For inspecting while debugging
	// TODO: remove once loading is implemented
	//std::vector<std::string> propertyNames = element.getPropertyNames();

	uint32_t numGaussians = (uint32_t)gPositionsX.size();

	assert(
		numGaussians == gPositionsY.size() &&
		numGaussians == gPositionsZ.size() &&

		numGaussians == gScalesX.size() &&
		numGaussians == gScalesY.size() &&
		numGaussians == gScalesZ.size() &&

		numGaussians == gRot0.size() &&
		numGaussians == gRot1.size() &&
		numGaussians == gRot2.size() &&
		numGaussians == gRot3.size() &&

		numGaussians == gOpacities.size()
	);

	this->gaussians.resize(numGaussians);
	/*for (uint32_t i = 0; i < this->numGaussians; ++i)
	{
		uint32_t randomIndex = rand() % gPositionsX.size();

		GaussianData gaussian{};
		gaussian.position = glm::vec4(
			gPositionsX[randomIndex],
			gPositionsY[randomIndex] * -1.0f,
			gPositionsZ[randomIndex],
			0.0f
		);
		gaussian.scale = glm::vec4(
			gScalesX[randomIndex] * 0.01f,
			gScalesY[randomIndex] * 0.01f,
			gScalesZ[randomIndex] * 0.01f,
			0.0f
		);
		gaussian.color = glm::vec4(
			std::abs(gFeature0[randomIndex]) * 0.28209479177387814f,
			std::abs(gFeature1[randomIndex]) * 0.28209479177387814f,
			std::abs(gFeature2[randomIndex]) * 0.28209479177387814f,
			gOpacities[randomIndex]
		);

		// Apply
		outputGaussianData[i] = gaussian;

		// Remove
		gPositionsX.erase(gPositionsX.begin() + randomIndex);
		gPositionsY.erase(gPositionsY.begin() + randomIndex);
		gPositionsZ.erase(gPositionsZ.begin() + randomIndex);

		gScalesX.erase(gScalesX.begin() + randomIndex);
		gScalesY.erase(gScalesY.begin() + randomIndex);
		gScalesZ.erase(gScalesZ.begin() + randomIndex);

		gFeature0.erase(gFeature0.begin() + randomIndex);
		gFeature1.erase(gFeature1.begin() + randomIndex);
		gFeature2.erase(gFeature2.begin() + randomIndex);

		gOpacities.erase(gOpacities.begin() + randomIndex);
	}*/
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
		gaussian.color = glm::vec4(
			0.5f + gFeature0[i] * 0.28209479177387814f,
			0.5f + gFeature1[i] * 0.28209479177387814f,
			0.5f + gFeature2[i] * 0.28209479177387814f, // TODO: evaluate SH in shader
			(1.0f / (1.0f + std::exp(-gOpacities[i])))
		);

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

	// Write ply for experimentation
	{
		happly::PLYData outPly;

		outPly.addElement(names[0], count + 1);
		happly::Element& outElement = outPly.getElement(names[0]);

		pushPlyProperty<float>(element, outElement, "x", 0.1f);
		pushPlyProperty<float>(element, outElement, "y", -0.1f);
		pushPlyProperty<float>(element, outElement, "z", -0.8f);
		pushPlyProperty<float>(element, outElement, "nx", 1.0f);
		pushPlyProperty<float>(element, outElement, "ny", 0.0f);
		pushPlyProperty<float>(element, outElement, "nz", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_dc_0", 5.0f);
		pushPlyProperty<float>(element, outElement, "f_dc_1", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_dc_2", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_0", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_1", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_2", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_3", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_4", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_5", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_6", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_7", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_8", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_9", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_10", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_11", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_12", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_13", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_14", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_15", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_16", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_17", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_18", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_19", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_20", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_21", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_22", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_23", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_24", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_25", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_26", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_27", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_28", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_29", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_30", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_31", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_32", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_33", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_34", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_35", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_36", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_37", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_38", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_39", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_40", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_41", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_42", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_43", 0.0f);
		pushPlyProperty<float>(element, outElement, "f_rest_44", 0.0f);
		pushPlyProperty<float>(element, outElement, "opacity", 4.0f);
		pushPlyProperty<float>(element, outElement, "scale_0", -1.0f);
		pushPlyProperty<float>(element, outElement, "scale_1", -1.0f);
		pushPlyProperty<float>(element, outElement, "scale_2", -1.0f);
		pushPlyProperty<float>(element, outElement, "rot_0", 1.0f);
		pushPlyProperty<float>(element, outElement, "rot_1", 0.0f);
		pushPlyProperty<float>(element, outElement, "rot_2", 0.0f);
		pushPlyProperty<float>(element, outElement, "rot_3", 0.0f);

		outPly.write(
			"D:/DownloadedAssets/GaussianFiles/train_modified/point_cloud/iteration_7000/point_cloud.ply", 
			happly::DataFormat::Binary
		);
	}
}
