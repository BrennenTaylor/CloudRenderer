#ifndef RANDOM_NUMBER_SUPPORT_HLSL
#define RANDOM_NUMBER_SUPPORT_HLSL

uint rand_lcg(uint rngState)
{
    // LCG values from Numerical Recipes
    return 1664525 * rngState + 1013904223;
}

uint rand_xorshift(uint rngState)
{
    // Xorshift algorithm from George Marsaglia's paper
    rngState ^= (rngState << 13);
    rngState ^= (rngState >> 17);
    rngState ^= (rngState << 5);
    return rngState;
}

uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

#endif