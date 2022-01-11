#pragma once

#include <FMath/FMath.h>
#include <Input/InputState.h>

#include <DirectXMath.h>

using namespace DirectX;

namespace Farlor
{
    // Defines the interface to a camera
    class Camera
    {
    public:
        Camera();
        virtual ~Camera();

        // Sets projection matrix parameters
        void SetLens(float fovY, float aspect, float zNear, float zFar);

        void Update(float dt, const InputState* pInputState);

        // Updates based on yaw, pitch and roll
        // This should be called after yaw, pitch and roll values are set
        void UpdateViewMatrix();
        void UpdateProjMatrix();

        inline const Vector3 GetWorldPosition() const
        {
            XMFLOAT4 worldPos;
            XMStoreFloat4(&worldPos, camPosition);
            return Farlor::Vector3(worldPos.x, worldPos.y, worldPos.z);
        }

        inline const Vector3 GetWorldTarget() const
        {
            XMFLOAT4 worldPos;
            XMStoreFloat4(&worldPos, camTarget);
            return Farlor::Vector3(worldPos.x, worldPos.y, worldPos.z);
        }

        inline const Vector3 GetWorldUp() const
        {
            XMFLOAT4 worldPos;
            XMStoreFloat4(&worldPos, camUp);
            return Farlor::Vector3(worldPos.x, worldPos.y, worldPos.z);
        }

        inline const Matrix4x4& GetView() const
        {
            return m_view;
        }

        inline const Matrix4x4& GetProj() const
        {
            return m_proj;
        }

        inline float GetNearZ() { return m_zNear; }
        inline float GetFarZ() { return m_zFar; }
        inline float GetAspect() { return m_aspect; }
        inline float GetFOV() { return m_fov; }


        inline bool MovedInFrame()
        {
            return m_movedInFrame;
        }

    private:
        XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR camPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

        XMMATRIX camRotationMatrix;

        float moveLeftRight = 0.0f;
        float moveBackForward = 0.0f;
        float moveUpDown = 0.0f;

        float camYaw = 0.0f;
        float camPitch = 0.0f;

        bool m_movedInFrame;

        // Projection parameters
        float m_zNear;
        float m_zFar;
        float m_aspect;
        float m_fov; // In radians

        Matrix4x4 m_view;
        Matrix4x4 m_proj;
    };
}