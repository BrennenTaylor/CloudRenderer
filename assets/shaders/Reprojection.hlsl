#ifndef REPROJECTION_HLSL
#define REPROJECTION_HLSL

cbuffer CamNew : register(b0)
{
    float4x4 NewView;
    float4x4 NewProj;
    float4 NewEye;
    float2 NewFov;
    float2 _NewPad;
};

cbuffer CamOld : register(b1)
{
    float4x4 NewView;
    float4x4 NewProj;
    float4 NewEye;
    float2 NewFov;
    float2 _NewPad;
};

cbuffer TimeBuffer : register(b2)
{
    float2 Time;
    float2 _TimePad;
};

void main()
{
    //3 options: - do a full resolution launch and calculate for every pixel and overwrite the value of one pixel with an actual raymarch
    //           - do all the work for 15 reprojection pixel fills on one thread
    //           - do full resolution but only work for 15 pixels and terminate early for the one that will be filled by raymarching
    //The last one is the best but it is hard to determine which pixel to skip --> equate the pixel selected with the regular invocation ID --> to determine which to skip
    ivec2 dim = imageSize(currentFrameResultImage);
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / dim;
    //Do not invert uv's because we already do this in the cloudCompute shader and so things are stored in accordance to openGL convention of uv's

    int pixelID = frameCountMod16;

	vec3 eyePos = -camera.eye.xyz;
	Ray ray = castRay(uv, eyePos, camera.view, camera.tanFovBy2, pixelID, dim);

	// Hit the inner sphere with the ray you just found to get some basis world position along your current ray
	vec3 earthCenter = eyePos;
	earthCenter.y = -EARTH_RADIUS; //move earth below camera
	Intersection atmosphereInnerIsect = raySphereIntersection(ray.origin, ray.direction, earthCenter, ATMOSPHERE_RADIUS_INNER);

    vec3 oldCameraRayDir = normalize((cameraOld.view * vec4(atmosphereInnerIsect.point, 1.0f)).xyz);

    // We have a normalized ray that we need to convert into uv space
    // We can achieve this by multiplying the xy componentes of the ray by the camera's Right and Up basis vectors  and scaling it down to a 0 to 1 range
    // In other words the ray goes from camera space <x,y,z> to uv space <u,v,1> using the R U F basis that defines the camera
    oldCameraRayDir /= -oldCameraRayDir.z; //-z because in camera space we look down negative z and so we don't want
                                           //to divide by a negative number --> that would make our uv's negative
    float old_u = (oldCameraRayDir.x / (camera.tanFovBy2.x)) * 0.5 + 0.5;
    float old_v = (oldCameraRayDir.y / (camera.tanFovBy2.y)) * 0.5 + 0.5;
    vec2 old_uv = vec2(old_u, old_v); //if old_uv is out of range -> the texture sampler simply returns black --> keep in mind when porting

    // if(old_u > 1.0f || old_u < 0.0f || old_v > 1.0f || old_v < 0.0f)
    // {
    //     imageStore( currentFrameResultImage, ivec2(gl_GlobalInvocationID.xy), vec4(0,0,1,0) );
    //     return;
    // }

    vec2 motionVec = old_uv - uv;
    vec4 blurColor = vec4(0.0);
    vec2 blurOffset = vec2(0.0);
    vec2 imageUV = round(old_uv * dim);

    for(int i=0; i<NUM_MOTION_BLUR_SAMPLES; i++)
    {
        blurOffset = motionVec * (float(i)/float(NUM_MOTION_BLUR_SAMPLES));
        imageUV = round( (old_uv - blurOffset) * dim);
        blurColor += imageLoad(previousFrameResultImage, clamp(ivec2(imageUV), ivec2(0, 0), ivec2(dim.x - 1,  dim.y - 1)));
    }
    blurColor /= NUM_MOTION_BLUR_SAMPLES;

    imageStore( currentFrameResultImage, ivec2(gl_GlobalInvocationID.xy), blurColor );
}

#endif