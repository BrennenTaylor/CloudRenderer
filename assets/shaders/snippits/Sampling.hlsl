#include "./assets/shaders/snippits/Constants.hlsl"

float3 UniformSampleHemisphere(float3 normal, float randU, float randV)
{
    float u = randU;
    float v = randV;

    float theta = 2.0f * F_PI * u;
    float phi = acos(2.0f * v - 1.0f);

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float3 dir = { cosTheta * sinPhi, sinTheta * sinPhi, cosPhi};

    // If our vector is facing the wrong way vs the normal, flip it!
    if (dot(dir, normal) <= 0.0f)
    {
        dir *= -1.0f;
    }

    return normalize(dir);
};