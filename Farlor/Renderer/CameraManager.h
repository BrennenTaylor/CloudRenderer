#pragma once

#include "Camera.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Farlor
{
    class CameraManager
    {
    public:
        CameraManager();
        ~CameraManager();

        Camera* RegisterCamera(std::string& cameraName, std::unique_ptr<Camera> upCamera);


        Camera* GetCurrentCamera() const
        {
            return m_pCurrentCamera;
        }

        Camera* SetMainCamera(std::string& cameraName);

    private:
        Camera* m_pCurrentCamera;
        std::unordered_map<std::string, std::unique_ptr<Camera>> m_registeredCameras;
    };
}