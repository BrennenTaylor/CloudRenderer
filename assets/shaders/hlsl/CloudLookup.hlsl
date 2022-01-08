float Remap(float origVal, float origMin, float origMax, float newMin, float newMax)
{
    return newMin + (((origVal - origMin) / (origMax - origMin)) * (newMax - newMin));
}