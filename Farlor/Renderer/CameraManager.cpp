#include "CameraManager.h"

namespace Farlor
{
    CameraManager::CameraManager()
        : m_pCurrentCamera{ nullptr }
        , m_registeredCameras{}
    {
    }

    CameraManager::~CameraManager()
    {
    }

    Camera* CameraManager::RegisterCamera(std::string& cameraName, std::unique_ptr<Camera> upCamera)
    {
        auto iter = m_registeredCameras.find(cameraName);
        // Defines the policy for what happens if a camera alreay is registerd with that name
        // Currently, we simply replace and the old camera will be deleted
        if (iter != m_registeredCameras.end())
        {
            iter->second = std::move(upCamera);
            return iter->second.get();
        }

        Camera* pCamera = upCamera.get();
        m_registeredCameras.insert(std::make_pair(cameraName, std::move(upCamera)));
        return pCamera;
    }

    // Sets the current camera and returns the value its set to for easy retrieval
    // Nulls out the current camera if the correct camera is not set
    Camera* CameraManager::SetMainCamera(std::string& cameraName)
    {
        auto iter = m_registeredCameras.find(cameraName);
        if (iter == m_registeredCameras.end())
        {
            m_pCurrentCamera = nullptr;
            return m_pCurrentCamera;
        }

        m_pCurrentCamera = iter->second.get();
        return m_pCurrentCamera;
    }
}