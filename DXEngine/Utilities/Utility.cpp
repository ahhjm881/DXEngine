#include "Utility.h"

#include <cmath>
#include <random>

float Math::AngleFromXY(float x, float y)
{
    float theta = std::atan2(y, x);
    theta = theta < 0.0f ? theta + DirectX::XM_2PI : theta;
    return theta;
}