#ifndef CLOUDTRACE_HLSL
#define CLOUDTRACE_HLSL

#include "CloudParams.hlsl"
#include "Geometry.hlsl"
#include "CloudLighting.hlsl"

#include "CloudLookup.hlsl"

cbuffer NewCamera : register(b0)
{
    float3 NewCameraPos;
    uint NewScreenWidth;
    float3 NewCameraTarget;
    uint NewScreenHeight;
    float3 NewWorldUp;
    float NewFOV_Horizontal;
};

cbuffer OldCamera : register(b1)
{
    float3 OldCameraPos;
    uint OldScreenWidth;
    float3 OldCameraTarget;
    uint OldScreenHeight;
    float3 OldWorldUp;
    float OldFOV_Horizontal;
};

cbuffer TimeValues : register(b2)
{
    float DeltaTime;
    float TotalTime;
    float2 _TimePad;
}

Texture3D lowFreqTex : register(t0);
Texture3D highFreqTex : register(t1);
Texture2D curlNoiseTex : register(t2);
Texture2D weatherMapTex : register(t3);

SamplerState textureSampler : register(s0);

// Ok, we need some input output buffers as well
RWStructuredBuffer<float4> CloudBuffer : register(u0);

// fractional value for sample position in the cloud layer
float GetHeightFractionForPoint(float3 inPosition, float3 earthCenter, float3 startPosOnInnerShell, float3 rayDir, float3 eye)
{
    // // get global fractional position in cloud zone
    // // Should be inPosition.y
    // float distAboveSurface = length(inPosition - earthCenter) - EARTH_RADIUS;
    // float height_fraction = (distAboveSurface - ATMOSPHERE_RADIUS_INNER ) / (ATMOSPHERE_RADIUS_OUTER - ATMOSPHERE_RADIUS_INNER);
    // return saturate(height_fraction);

    float lengthOfRayfromCamera = length(inPosition - eye);
    float lengthOfRayToInnerShell = length(startPosOnInnerShell - eye);
    float3 pointToEarthDir = normalize(inPosition - earthCenter);
    // assuming RayDir is normalised
	float cosTheta = dot(rayDir, pointToEarthDir);

    // CosTheta is an approximation whose error gets relatively big near the horizon and could lead to problems.
    // However, the actual calculationis involve a lot of trig and thats expensive;
    // No longer drawing clouds that close to the horizon and so the cosTheta Approximation is fine

    if (inPosition.y < eye.y)
    {
        return 0.0f;
    }

	float numerator = abs(cosTheta * (lengthOfRayfromCamera - lengthOfRayToInnerShell));
    return numerator / (ATMOSPHERE_RADIUS_OUTER - ATMOSPHERE_RADIUS_INNER);
    // return clamp( length(point.y - projectedPos.y) / ATMOSPHERE_THICKNESS, 0.0, 1.0);
}


float DensityHeightAtPoint(float densityHeight, float3 weather)
{
    float stratus = Remap(densityHeight, 0.0f, 0.1f, 0.0f, 1.0f)
        * Remap(densityHeight, 0.2f, 0.3f, 1.0f, 0.0f);

    float strato = Remap(densityHeight, 0.0f, 0.2f, 0.0f, 1.0f)
        * Remap(densityHeight, 0.45f, 0.6f, 1.0f, 0.0f);

    float cumulus = Remap(densityHeight, 0.0f, 0.1f, 0.0f, 1.0f)
        * Remap(densityHeight, 0.7f, 0.95f, 1.0f, 0.0f);

    // Full stratus at 0.0
    // Full strato at 0.5
    // Full cumulus at 1.0

    // Map 0->1 to 0->2
    float stratusToStratoAmount = clamp(weather.g * 2.0f, 0.0f, 1.0f);
    // Map 0->1 to -1.0->1.0
    float stratoToCumulusAmount = clamp((weather.g - 0.5f) * 2.0f, 0.0f, 1.0f);

    float stratusToStratoInterp = lerp(stratus, strato, stratusToStratoAmount);
    float stratoToCumulusInterp = lerp(strato, cumulus, stratoToCumulusAmount);

    float cloudTypeHeightWeight = lerp(stratusToStratoInterp, stratoToCumulusInterp, densityHeight);

    return cloudTypeHeightWeight;
}

// Does not optimize by only grabbing the low freq value
// Assumes that the y value of p is from the earth center!
float SampleCloudDensity(float3 p, float3 earthCenter, float3 weather_data, bool doCheaply, float3 startPosOnInnerShell, float3 rayDir, float3 eye)
{
    // get height fraction
    float height_fraction = GetHeightFractionForPoint(p, earthCenter, startPosOnInnerShell, rayDir, eye);
    
    // wind settings
    float3 wind_direction = float3(1.0, 0.0, 0.0);
    float cloud_speed = 10.0;

    // cloud_top offset - push the tops of the clouds along this wind direction by this many units.
    float cloud_top_offset = 500.0;

    // skew in wind direction
    p += height_fraction * wind_direction * cloud_top_offset;

    //animate clouds in wind direction and add a small upward bias to the wind direction
    p += (wind_direction + float3(0.0, 0.1, 0.0) ) * TotalTime * cloud_speed * 100.0f;

    // read the low frequency Perlin-Worley and Worley noises
    float4 low_frequency_noises = lowFreqTex.SampleLevel(textureSampler, p.xyz / 10000.0f, 0).rgba;

    // build an fBm out of  the low frequency Worley noises that can be used to add detail to the Low frequency Perlin-Worley noise
    float low_freq_fBm = ( low_frequency_noises.g * 0.625 ) + ( low_frequency_noises.b * 0.25 ) + ( low_frequency_noises.a * 0.125 );

    // define the base cloud shape by dilating it with the low frequency fBm made of Worley noise.
    float base_cloud = Remap( low_frequency_noises.r, - ( 1.0 -  low_freq_fBm), 1.0, 0.0, 1.0 );

    // Get the density-height gradient using the density-height function
    float density_height_gradient = DensityHeightAtPoint(height_fraction, weather_data);

    // apply the height function to the base cloud shape
    base_cloud *=  density_height_gradient;

    // cloud coverage is stored in the weather_data’s red channel.
    float cloud_coverage = weather_data.r;

    //Use remapper to apply cloud coverage attribute
    float base_cloud_with_coverage  = Remap(base_cloud, cloud_coverage, 1.0, 0.0, 1.0); 

    //Multiply result by cloud coverage so that smaller clouds are lighter and more aesthetically pleasing.
    base_cloud_with_coverage *= cloud_coverage;

    //define final cloud value
    float final_cloud = base_cloud_with_coverage;

    // only do detail work if we are taking expensive samples!
    if(!doCheaply)
    {

        // add some turbulence to bottoms of clouds using curl noise.  Ramp the effect down over height and scale it by some value (200 in this example)
        float2 curl_noise = curlNoiseTex.SampleLevel(textureSampler, p.xy / 8000.0f, 0).rg;
        p.xz += curl_noise.rg * (1.0 - height_fraction) * 200.0;

        // sample high-frequency noises
        float3 high_frequency_noises = highFreqTex.SampleLevel(textureSampler, p.xyz / 60000.0f, 0).rgb;

        // build High frequency Worley noise fBm
        float high_freq_fBm = ( high_frequency_noises.r * 0.625 ) + ( high_frequency_noises.g * 0.25 ) + ( high_frequency_noises.b * 0.125 );

        // get the height_fraction for use with blending noise types over height
        float height_fraction = GetHeightFractionForPoint(p, earthCenter, startPosOnInnerShell, rayDir, eye);

        // transition from wispy shapes to billowy shapes over height
        float high_freq_noise_modifier = lerp(high_freq_fBm, 1.0 - high_freq_fBm, saturate(height_fraction * 10.0));

        // erode the base cloud shape with the distorted high frequency Worley noises.
        final_cloud = Remap(base_cloud, high_freq_noise_modifier * 0.2, 1.0, 0.0, 1.0);
    }
    return max(final_cloud, 0.0f);
}

float GetLightEnergy( float3 p, float height_fraction, float lightDensity, float cloudDensity, float phase_probability, float cos_angle, float step_size, float brightness)
{
    // attenuation – difference from slides – reduce the secondary component when we look toward the sun.
    float primary_attenuation = exp(-lightDensity);
    float secondary_attenuation = exp(-lightDensity * 0.25) * 0.7;

    float atten = BeerLamb(lightDensity);

    // Is this correct? Curves didnt exist in code
    //float attenuation_probability = max(Remap(cos_angle, 0.7, 1.0, secondary_attenuation, secondary_attenuation * 0.25), primary_attenuation);

    // in-scattering – one difference from presentation slides – we also reduce this effect once light has attenuated to make it directional.
    float depth_probability = lerp( 0.05 + pow( cloudDensity, Remap( height_fraction, 0.3, 0.85, 0.5, 2.0 )), 1.0, saturate( lightDensity / step_size));
    float vertical_probability = pow( Remap( height_fraction, 0.07, 0.14, 0.1, 1.0 ), 0.8 );
    float in_scatter_probability = depth_probability * vertical_probability;

    float light_energy = atten * in_scatter_probability * phase_probability * brightness;

    return light_energy;
}


float4 PerformCloudMarch(Ray cloudRay,
    float3 earthCenter, float3 eye, Intersection innerInter,
    Intersection outerInter)
{
    int numSteps = 60;
    float tDist = outerInter.t - innerInter.t;
    float stepSize = tDist / numSteps;

    float3 startTracePos = cloudRay.origin + innerInter.t * normalize(cloudRay.direction);
    float3 traceDir = normalize(cloudRay.direction);

    float substinenceDensity = 0.1f;
    float coverageClip = 0.0f;

    float sunIntensity = 1.0f;
    float3 sunColor = float3(1.0f, 1.0f, 1.0f) * sunIntensity;

    float k = 0.9f;

    float3 sunPosition = float3(0.0f, EARTH_RADIUS * (4.0f + sin(TotalTime)), 0.0f);

    // Can probably optimize a little later
    float3 radiance = float3(0.0f, 0.0f, 0.0f);
    float3 transmittence = float3(1.0f, 1.0f, 1.0f);
    float totalDensity = 0.0f;
    for (int i = 0; i < numSteps; ++i)
    {
        float3 samplePoint = startTracePos + stepSize * i * traceDir;
        float3 weather = weatherMapTex.SampleLevel(textureSampler, samplePoint.xy / 60000.0f, 0).xyz;
        //weather.b = 0.0f;

        float3 lightDirection = normalize(sunPosition - samplePoint);
        float cosAngle = dot(normalize(cloudRay.direction), lightDirection);
        const float eccentricity = 0.6;
        const float silver_intensity = 0.7;
        const float silver_spread = 0.1;
        const float hgmVal = HGM(cosAngle, eccentricity, silver_intensity, silver_spread);

        float cloudDensity = SampleCloudDensity(samplePoint, earthCenter, weather, true, startTracePos, cloudRay.direction, eye) * substinenceDensity;

        // Only light the point if we have density at that location
        if (cloudDensity > 0.0f)
        {
            totalDensity += cloudDensity;

            // We also need to trace a light ray to the sun as well
            // We only trace 6 samples, super low
            int numLightSamples = 6;
            float lightDensity = 0.0f;
            float3 combinedColor = float3(0.0f, 0.0f, 0.0f);
            for (int l = 0; l < numLightSamples; ++l)
            {
                float3 lightSamplePos = samplePoint + lightDirection * stepSize * l * 1.0f;
                float3 lightWeather = weatherMapTex.SampleLevel(textureSampler, lightSamplePos.xy / 60000.0f, 0).xyz;

                float fullLightDensity = SampleCloudDensity(lightSamplePos, earthCenter, lightWeather, true, startTracePos, cloudRay.direction, eye) * substinenceDensity;
                lightDensity += fullLightDensity;

                float scaledLightDensity = exp(-1.0f * k * lightDensity);
                combinedColor += sunColor * scaledLightDensity * 0.8f;
            }

            float dt = exp(-1.0f * k * stepSize * cloudDensity);
            radiance += combinedColor * (1.0f - dt) * transmittence;
            transmittence *= dt;
        }
    }

    return float4(radiance, totalDensity);
}

// In meters
float3 planeEdgeLength = 1000.0f;

// The number of threads should be exposed as compile time defines
#ifdef USE_DEFAULT_THREAD_COUNTS

#define XThreadCount 32
#define YThreadCount 16
#define ZThreadCount 1

#else

#endif

// X : 32
// Y : 16
// Z : 1
// Total, Group Size : 512 threads
[numthreads(XThreadCount, YThreadCount, ZThreadCount)] void CSMain(uint3 dispatchThreadID
                                                                   : SV_DispatchThreadID, uint3 groupID
                                                                   : SV_GroupID) {
    // Seed the random number generator
    // Hash using the thread id, then use wang_hash to make the seed go wide
    uint index = (dispatchThreadID.x + dispatchThreadID.y * NewScreenWidth);

    int pixelX = dispatchThreadID.x;
    int pixelY = dispatchThreadID.y;

    int2 dim = int2(NewScreenWidth, NewScreenWidth);

    float2 uv = float2(pixelX, pixelY) / dim;

    const float aspectRatio = (float)(NewScreenWidth) / (float)(NewScreenHeight);

    float3 camForward = normalize(NewCameraTarget - NewCameraPos);
    float3 camRight = -1.0f * normalize(cross(camForward, NewWorldUp));
    float3 camUp = -1.0f * normalize(cross(camForward, camRight));

    float horFov = NewFOV_Horizontal;
    float vertFov = horFov / aspectRatio;
    float nearDistance = 0.1f;

    float windowTop = tan(vertFov / 2.0f) * nearDistance;
    float windowRight = tan(horFov / 2.0f) * nearDistance;

    uint xVal = dispatchThreadID.x;
    uint yVal = dispatchThreadID.y;

    float u = (float)xVal / float(NewScreenWidth);
    float v = (float)yVal / float(NewScreenHeight);

    // Transform to [-1, 1] space from [0, 1] space
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    float3 skyBlue = float3(0.529412f, 0.807843f, 0.921569f);
    float3 primaryRayOrigin = {0.0f, 0.0f, 0.0f};
    float3 primaryRayDirection = {0.0f, 0.0f, 0.0f};

    primaryRayOrigin = NewCameraPos + camForward * nearDistance;
    primaryRayOrigin = primaryRayOrigin + camRight * windowRight * u;
    primaryRayOrigin = primaryRayOrigin + camUp * windowTop * v;
    primaryRayDirection = normalize(primaryRayOrigin - NewCameraPos);

    Ray cloudRay;
    cloudRay.origin = NewCameraPos;
    cloudRay.direction = primaryRayDirection;


    float3 finalColor = float3(0.0f, 0.0f, 0.0f);

    bool diskInter = RayDiskIntersection(float3(0.0f, -1.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), 10000.0f, cloudRay); 
    if (diskInter)
    {
        finalColor = float3(0.333333, 0.419608, 0.184314);
        CloudBuffer[index] = float4(finalColor, 1.0f);
        return;
    }

    float horizonAngle = dot(NewWorldUp, cloudRay.direction);
    const float backgroundColorMultiplier = 1.0f;
    float3 transitionGradient = float3(1.0f, 1.0f, 1.0f);
    const float cloudFadeOutPoint = 0.05f;


    // Below horizon
    // if (horizonAngle < 0.0)
    // {
    //     // Simply draw out black
    //     finalColor = float3(0.0f, 0.0f, 0.0f);
    //     CloudBuffer[index] = float4(finalColor, 1.0f);
    //     return;
    // }
    // else
    {
        // Can we do a horizon color?
        finalColor = skyBlue * backgroundColorMultiplier;
    }

    // Perform the ray march
    //float3 earthCenter = NewCameraPos - NewWorldUp * EARTH_RADIUS;
    float3 earthCenter = float3(0.0f, 0.0f, 0.0f) - NewWorldUp * EARTH_RADIUS;

    Intersection innerInter = RaySphereIntersection(cloudRay.origin, cloudRay.direction,
        earthCenter, ATMOSPHERE_RADIUS_INNER + EARTH_RADIUS);

    Intersection outerInter = RaySphereIntersection(cloudRay.origin, cloudRay.direction,
        earthCenter, ATMOSPHERE_RADIUS_OUTER + EARTH_RADIUS);

    // Ray March
    float4 rayMarchResult = PerformCloudMarch(cloudRay, earthCenter, NewCameraPos, innerInter, outerInter);

    finalColor = lerp(finalColor, rayMarchResult.xyz, rayMarchResult.w);

//Global Defines for Debug Views
#define CLOUD_DENSITY 0
#define TEXTURE_LOW_FREQ 0
#define TEXTURE_HIGH_FREQ 0
#define TEXTURE_CURL_NOISE 0
#define TEXTURE_WEATHER_NOISE 0

    // Begin debug renders
#if TEXTURE_LOW_FREQ
    float4 lowFrequencyNoises = lowFreqTex.SampleLevel(textureSampler, float3(uv, sin(TotalTime)), 0);
    finalColor = float4(lowFrequencyNoises);
#elif TEXTURE_HIGH_FREQ
    float4 highFrequencyNoise = highFreqTex.SampleLevel(textureSampler, float3(uv, sin(TotalTime)), 0);
    finalColor = float4(highFrequencyNoise);
#elif TEXTURE_CURL_NOISE
    float4 curlNoise = curlNoiseTex.SampleLevel(textureSampler, uv, 0);
    finalColor = float4(curlNoise.xyz, 1.0f);
#elif TEXTURE_WEATHER_NOISE
    float4 weatherVal = weatherMapTex.SampleLevel(textureSampler, uv, 0);
    finalColor = float4(weatherVal.xyz, 1.0f);
#elif CLOUD_DENSITY
    finalColor = float4(float3(rayMarchResult.w, rayMarchResult.w, rayMarchResult.w), 1.0);
#endif

    // Write out the color to the correct spot in the color buffer
    CloudBuffer[index] = float4(finalColor, 1.0f);
}

#endif