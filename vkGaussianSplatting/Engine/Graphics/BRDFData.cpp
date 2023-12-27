#include "pch.h"
#include "BRDFData.h"
#include "../Dev/StrHelper.h"

bool BRDFData::createFromFile(const std::string& filePath)
{
	// Open file
	std::ifstream brdfFile(filePath);
	if (!brdfFile.is_open())
	{
		Log::error("Failed to open BRDF file: " + filePath);
		return false;
	}

	std::vector<std::string> splitStringResult;
	splitStringResult.reserve(5);

	std::vector<std::vector<RGB>>* currentCoeffs = &this->shCoefficients;

	// Read lines
	std::string line;
	while (std::getline(brdfFile, line))
	{
		// Switch to load in coefficients with cos term
		if (line.substr(0, 2) == "kc")
			currentCoeffs = &this->shCoefficientsCosTerm;

		// New set of SH coefficients
		if (line[0] == 'k')
		{
			currentCoeffs->push_back(std::vector<RGB>());
		}
		// New coefficient
		else if(line.size() != 0)
		{
			// Read 1 coefficient per color channel
			splitStringResult.clear();
			StrHelper::splitString(line, ' ', splitStringResult);

			// Create RGB
			RGB newCoefficient
			{
				std::stod(splitStringResult[0]),
				std::stod(splitStringResult[1]),
				std::stod(splitStringResult[2])
			};

			uint32_t currentShIndex = uint32_t(currentCoeffs->size()) - 1;
			(*currentCoeffs)[currentShIndex].push_back(newCoefficient);
		}
	}

	// Close file
	brdfFile.close();

	return true;
}
