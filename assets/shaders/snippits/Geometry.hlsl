#ifndef GEOMETRY_HLSL
#define GEOMETRY_HLSL

struct Ray
{
    float3 m_start;
    float m_pad0;
    float3 m_direction;
    float m_pad1;
};

struct Triangle
{
    float3 m_p0;
    float3 m_p1;
    float3 m_p2;
};

// Define intersection tests
bool RayIntersectTriangle(uniform Ray ray, uniform Triangle tri, out float t,  out float u,  out float v)
{
    bool result = false;
    float3 edge0 = tri.m_p1 - tri.m_p0;
    float3 edge1 = tri.m_p2 - tri.m_p0;

    float3 pvec = cross(ray.m_direction, edge1);
    float det = dot(edge0, pvec);

    if (det < 0.0001)
    {
        return false;
    }

    float invDet = 1.0f / det;

    float3 tvec = ray.m_start - tri.m_p0;
    u = dot(tvec, pvec) * invDet;
    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }

    float3 qvec = cross(tvec, edge0);
    v = dot(ray.m_direction, qvec) * invDet;
    if (v < 0.0f || (u + v) > 1.0f)
    {
        return false;
    }

    t = dot(edge1, qvec) * invDet;
    return true;
}

#endif