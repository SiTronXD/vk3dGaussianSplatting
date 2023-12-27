#pragma once

#include <vector>

struct RGB
{
	double rgb[3];
};

class BRDFData
{
private:
	// shCoefficients[wo][shIndex]
	std::vector<std::vector<RGB>> shCoefficients;
	std::vector<std::vector<RGB>> shCoefficientsCosTerm;

public:
	bool createFromFile(const std::string& filePath);

	inline const std::vector<std::vector<RGB>>& getShCoefficientSets() const { return this->shCoefficients; }
	inline const std::vector<std::vector<RGB>>& getShCoefficientCosSets() const { return this->shCoefficientsCosTerm; }
};