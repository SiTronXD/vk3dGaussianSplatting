#include "pch.h"
#include "SMath.h"

const float SMath::PI = 3.141592f;

float SMath::roundToThreeDecimals(float x)
{
    int xx = int(x * 1000.0f + 0.5f);

    return float(xx) / 1000.0f;
}
