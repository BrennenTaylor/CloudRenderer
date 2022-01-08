#ifndef RANDOM_NUMBERS_HLSL
#define RANDOM_NUMBERS_HLSL

#include "./assets/shaders/snippits/RandomNumberSupport.hlsl"

// Huge Assumption, requires the use of a random number buffer

#ifdef USE_DEFAULT_RNG_STATE_REGISTER
#define RNG_STATE_REGISTER u1
#else
// Compiler must define
#endif

RWStructuredBuffer<uint> randomBuffer : register(RNG_STATE_REGISTER);

float GetRandomUniformFloat(uint index)
{
    uint currentSeed = randomBuffer[index];
    uint newValue = rand_xorshift(currentSeed);
    randomBuffer[index] = newValue;
    return float(newValue) * (1.0 / 4294967296.0);
}

#endif