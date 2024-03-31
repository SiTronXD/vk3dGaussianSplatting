#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SMath
{
private:
	// Source: https://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
	static inline uint32_t mortonPartBy2(uint32_t x)
	{
		x &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
		x = (x ^ (x << 16)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x ^ (x <<  8)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x ^ (x <<  4)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x ^ (x <<  2)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
		return x;
	}

public:
	static const float PI;

	static float roundToThreeDecimals(float x);
	static inline uint32_t encodeZorderCurve(glm::uvec3 position)
	{
		// Only 10 bits are supported per component to produce 32 bit key
		assert(position.x < 1024);
		assert(position.y < 1024);
		assert(position.z < 1024);

		return (SMath::mortonPartBy2(position.z) << 2) + 
			(SMath::mortonPartBy2(position.y) << 1) + 
			SMath::mortonPartBy2(position.x);
	}
};