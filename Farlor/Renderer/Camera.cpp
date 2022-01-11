#include "Camera.h"



#include <DirectXMath.h>

namespace Farlor
{
    Camera::Camera()
        : m_movedInFrame{ false }
        , m_zNear { 0.01f }
        , m_zFar { 1000.0f }
        , m_aspect { 1.0f }
        , m_fov { 60.0f }
        //, m_rotationMatrix{ Matrix4x4::s_Identity }
        , m_view{Matrix4x4::s_Identity}
        , m_proj{ Matrix4x4::s_Identity }
    {
        UpdateViewMatrix();
        UpdateProjMatrix();
    }

    Camera::~Camera()
    {
    }

    void Camera::SetLens(float fov, float aspect, float zNear, float zFar)
    {
        // We have set the lense here
        m_fov = fov;
        m_aspect = aspect;
        m_zNear = zNear;
        m_zFar = zFar;

        UpdateProjMatrix();
    }

    void Camera::Update(float dt, const InputState& inputState)
    {
        m_movedInFrame = false;

        const float moveSpeed = 1000.0f;

        // Should move right
        if (inputState.m_keyboardButtonStates[KeyboardButtons::D].endedDown)
        {
            m_movedInFrame = true;
            moveLeftRight += moveSpeed * dt;
        }

        // Should move left
        if (inputState.m_keyboardButtonStates[KeyboardButtons::A].endedDown)
        {
            m_movedInFrame = true;
            moveLeftRight -= moveSpeed * dt;
        }

        // Should move up
        if (inputState.m_keyboardButtonStates[KeyboardButtons::Shift].endedDown)
        {
            m_movedInFrame = true;
            moveUpDown += moveSpeed * 5.0f * dt;
        }

        // Should move down
        if (inputState.m_keyboardButtonStates[KeyboardButtons::Ctrl].endedDown)
        {
            m_movedInFrame = true;
            moveUpDown -= moveSpeed * 5.0f * dt;
        }

        // Should move forward
        if (inputState.m_keyboardButtonStates[KeyboardButtons::W].endedDown)
        {
            m_movedInFrame = true;
            moveBackForward += moveSpeed * dt;
        }

        // Should move back
        if (inputState.m_keyboardButtonStates[KeyboardButtons::S].endedDown)
        {
            m_movedInFrame = true;
            moveBackForward -= moveSpeed * dt;
        }

        // Unused as of now, camera can simply strafe
        camYaw += inputState.m_mouseUpdateDelta.x * 0.001f;
        camPitch += inputState.m_mouseUpdateDelta.y * 0.001f;
            
        UpdateViewMatrix();
    }

    // Updates based on yaw, pitch and roll
    // This should be called after yaw, pitch and roll values are set
    void Camera::UpdateViewMatrix()
    {
        //Farlor::Vector3 angles(m_yaw, m_pitch, 0.0f);
        //m_rotationMatrix = Matrix4x4::RotationYawPitchRollMatrix(angles);

        //{
        //    Farlor::Vector4 tempWorldForward(m_worldForward.x, m_worldForward.y, m_worldForward.z, 1.0f);
        //    auto tempCamForward = m_rotationMatrix * tempWorldForward;
        //    m_camForward = Farlor::Vector3(tempCamForward.x, tempCamForward.y, tempCamForward.z);
        //    m_camForward.Normalize();
        //}
        //{
        //    Farlor::Vector4 tempWorldForward(m_worldRight.x, m_worldRight.y, m_worldRight.z, 1.0f);
        //    auto tempCamForward = m_rotationMatrix * tempWorldForward;
        //    m_camRight = Farlor::Vector3(tempCamForward.x, tempCamForward.y, tempCamForward.z);
        //    m_camRight.Normalize();
        //}
        //{
        //    Farlor::Vector4 tempWorldForward(m_worldUp.x, m_worldUp.y, m_worldUp.z, 1.0f);
        //    auto tempCamForward = m_rotationMatrix * tempWorldForward;
        //    m_camUp = Farlor::Vector3(tempCamForward.x, tempCamForward.y, tempCamForward.z);
        //    m_camUp.Normalize();
        //}

        //std::cout << "Cam Pos: " << m_worldPosition << std::endl;
        //std::cout << "Cam Target: " << m_worldPosition + m_camForward << std::endl;

        //m_view = Matrix4x4::LookAtLH(m_worldPosition, m_worldPosition + m_camForward, m_worldUp).Transposed();

        camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
        camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
        camTarget = XMVector3Normalize(camTarget);

        XMMATRIX RotateYTempMatrix;
        RotateYTempMatrix = XMMatrixRotationY(camYaw);

        camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
        camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
        camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);


        camPosition += XMVectorScale(camRight, moveLeftRight);
        camPosition += XMVectorScale(camForward, moveBackForward);
        camPosition += XMVectorScale(camUp, moveUpDown);

        moveLeftRight = 0.0f;
        moveBackForward = 0.0f;
        moveUpDown = 0.0f;

        camTarget = camPosition + camTarget;

        auto camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

        XMFLOAT4X4 tempValue;
        XMStoreFloat4x4(&tempValue, camView);

        Matrix4x4 value;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                value.m_rows[i][j] = tempValue.m[i][j];
            }
        }
        // TODO: Why does this need to be Transposed?
        m_view = value.Transposed();
    }

    // Expects radians
    void Camera::UpdateProjMatrix()
    {
        m_proj = Matrix4x4::PerspectiveLHFOV(m_fov, m_aspect, m_zNear, m_zFar);
    }
}