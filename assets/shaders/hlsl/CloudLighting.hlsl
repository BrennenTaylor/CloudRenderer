#ifndef CLOUDLIGHTING_HLSL
#define CLOUDLIGHTING_HLSL

#include "CloudParams.hlsl"

float HG(float cosAngle, float ecc)
{
    return ((1.0 - ecc * ecc)
        / pow((1.0 + ecc * ecc - 2.0 * ecc * cosAngle), 3.0 / 2.0))
        / 4.0 * PI;
}

float HGM(float cosAngle, float ecc, float silverInt, float silverSpread)
{
    float first = HG(cosAngle, ecc);
    float second = HG(cosAngle, 0.99 - silverSpread);
    return max(first, silverInt * second);
}

float BeerLamb(float density)
{
    float first = exp(-density);
    float second = exp(-density * 0.25f) * 0.7f;
    return max(first, second);
}

#endif