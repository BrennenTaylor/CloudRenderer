#include "Game.h"

#include "FixedUpdate.h"
#include "Scene.h"
#include "Timer.h"

#include "../Utils/TinyXmlUtil.h"

#include "../Log.h"

#include "btBulletDynamicsCommon.h"

#include <D3D11SpatiotemporalFilter.h>
#include <Input/InputStateManager.h>
#include <ObjMesh.h>
#include <IWindow.h>
#include <Transform.h>
#include <TriMesh.h>
#include <WindowFactory.h>

#include <FMath/FMath.h>
#include <tinyxml2.h>

#include <cassert>
#include <sstream>

namespace Farlor
{
    Game::Game()
        : m_running{true}
        , m_pGameWindow{ nullptr }
        , m_upInputStateManager{nullptr}
        , m_upGameTimer{nullptr}
        , m_upRenderer(nullptr)
        , m_upGraphicsBackend(nullptr)
        , m_cameraManager()
        , m_resourceDir{ "" }
        , m_nextGameObjectId{0}
        , m_gameObjectNameLookup{}
        , m_geometryCache()
    {
    }

    Game::~Game()
    {
    }

    bool Game::Initialize(const std::string& resourceDir)
    {
        m_resourceDir = resourceDir;

        WindowFactory windowFactory;
        m_pGameWindow = windowFactory.CreateWindowInstance();
        ASSERT(m_pGameWindow, "Failed to create a window");

        m_upInputStateManager = std::make_unique<InputStateManager>();

        auto windowEventCallback = [this](WindowEvent* pEvent)
        {
            if (!pEvent)
            {
                return;
            }

            switch (pEvent->type)
            {
            // Window Events
            case WindowEventType::WindowClosedEvent:
            {
                // We want to exit the app!!
                m_running = false;
            } break;

            case WindowEventType::WindowResizeEvent:
            {
            } break;

            // Mouse Events
            // Calculate the delta movement then reset the cursor position
            case WindowEventType::MouseMoveEvent:
            {
                auto windowCenter = m_pGameWindow->GetWindowCenter();

                // NOTE: MUST be signed
                int32_t deltaX = static_cast<int32_t>(pEvent->WindowEventArgs.mouseMoveEventArgs.xPos - windowCenter.x);
                int32_t deltaY = static_cast<int32_t>(pEvent->WindowEventArgs.mouseMoveEventArgs.yPos - windowCenter.y);

                m_upInputStateManager->AddMouseDelta(deltaX, deltaY);
                m_pGameWindow->SetCursorPos(static_cast<int32_t>(windowCenter.x), static_cast<int32_t>(windowCenter.y));
            } break;

            case WindowEventType::MouseButtonEvent:
            {
                bool state = (pEvent->WindowEventArgs.mouseButtonEventArgs.buttonState == WindowMouseButtonState::Down)
                    ? true : false;

                switch (pEvent->WindowEventArgs.mouseButtonEventArgs.button)
                {
#define ADD_MOUSE_CASE(Button) \
    case WindowMouseButtons::##Button:\
    {\
        m_upInputStateManager->SetMouseState(MouseButtons::##Button, state);\
    } break;

                // Special
                ADD_MOUSE_CASE(Left);
                ADD_MOUSE_CASE(Right);
                ADD_MOUSE_CASE(Center);

                default:
                {
                } break;
                };
            } break;

            // Keyboard Events
            case Farlor::WindowEventType::KeyboardButtonEvent:
            {
                bool state = (pEvent->WindowEventArgs.keyboardButtonEventArgs.buttonState == Farlor::WindowKeyboardButtonState::Down)
                    ? true : false;

                switch (pEvent->WindowEventArgs.keyboardButtonEventArgs.button)
                {
#define ADD_KEYBOARD_CASE(Key) \
    case Farlor::WindowKeyboardButton::##Key:\
    {\
        m_upInputStateManager->SetKeyboardState(Farlor::KeyboardButtons::##Key, state);\
    } break;

                // Special
                ADD_KEYBOARD_CASE(Backspace);
                ADD_KEYBOARD_CASE(Tab);
                ADD_KEYBOARD_CASE(Return);
                ADD_KEYBOARD_CASE(Shift);
                ADD_KEYBOARD_CASE(Ctrl);
                ADD_KEYBOARD_CASE(Alt);
                ADD_KEYBOARD_CASE(Space);
                ADD_KEYBOARD_CASE(Escape);

                // Arrow Keys
                ADD_KEYBOARD_CASE(LeftArrow);
                ADD_KEYBOARD_CASE(UpArrow);
                ADD_KEYBOARD_CASE(RightArrow);
                ADD_KEYBOARD_CASE(DownArrow);

                // Numbers
                ADD_KEYBOARD_CASE(Zero);
                ADD_KEYBOARD_CASE(One);
                ADD_KEYBOARD_CASE(Two);
                ADD_KEYBOARD_CASE(Three);
                ADD_KEYBOARD_CASE(Four);
                ADD_KEYBOARD_CASE(Five);
                ADD_KEYBOARD_CASE(Six);
                ADD_KEYBOARD_CASE(Seven);
                ADD_KEYBOARD_CASE(Eight);
                ADD_KEYBOARD_CASE(Nine);

                // Alphabet
                ADD_KEYBOARD_CASE(A);
                ADD_KEYBOARD_CASE(B);
                ADD_KEYBOARD_CASE(C);
                ADD_KEYBOARD_CASE(D);
                ADD_KEYBOARD_CASE(E);
                ADD_KEYBOARD_CASE(F);
                ADD_KEYBOARD_CASE(G);
                ADD_KEYBOARD_CASE(H);
                ADD_KEYBOARD_CASE(I);
                ADD_KEYBOARD_CASE(J);
                ADD_KEYBOARD_CASE(K);
                ADD_KEYBOARD_CASE(L);
                ADD_KEYBOARD_CASE(M);
                ADD_KEYBOARD_CASE(N);
                ADD_KEYBOARD_CASE(O);
                ADD_KEYBOARD_CASE(P);
                ADD_KEYBOARD_CASE(Q);
                ADD_KEYBOARD_CASE(R);
                ADD_KEYBOARD_CASE(S);
                ADD_KEYBOARD_CASE(T);
                ADD_KEYBOARD_CASE(U);
                ADD_KEYBOARD_CASE(V);
                ADD_KEYBOARD_CASE(W);
                ADD_KEYBOARD_CASE(X);
                ADD_KEYBOARD_CASE(Y);
                ADD_KEYBOARD_CASE(Z);
                ADD_KEYBOARD_CASE(Tilda);
                default:
                {
                } break;
                };
            } break;

            };
            // NOTE: This requires the same c-runtime to be linked.
            delete pEvent;
        };

        m_pGameWindow->SetWindowEventCallback(windowEventCallback);

        m_upGameTimer = std::make_unique<Farlor::Timer>();

        // This needs to be generalized for other render types
        m_upRenderer = std::make_unique<Renderer>(m_cameraManager, m_resourceDir);

        float windowScale = 0.5f;
        const int width = 1920 * windowScale;
        const int height = 1080 * windowScale;
        const bool fullscreen = false;
        const bool startShown = true;
        m_pGameWindow->Initialize(width, height, fullscreen, startShown);
        // NOTE: Window must be shown before the renderer can be initialized
        m_pGameWindow->ShowGameWindow();

        m_upGraphicsBackend = std::make_unique<Farlor::D3D11SpatiotemporalFilterBackend>(*m_upRenderer);
        m_upRenderer->Initialize(m_pGameWindow, m_upGraphicsBackend.get());

        // Setup camera
        std::unique_ptr<Camera> upCamera = std::make_unique<Camera>();
        const float fovInRad = 0.785398f * 2; // 45 degrees
        upCamera->SetLens(fovInRad, width / height, 1.0f, 1000.0f);
        m_cameraManager.RegisterCamera(std::string("Debug"), std::move(upCamera));
        m_cameraManager.SetMainCamera(std::string("Debug"));
        return true;
    }

    std::optional<std::unique_ptr<Scene>> Game::LoadSceneFarlor(const std::string& envFilename)
    {
        uint32_t gameObjectId = 1;
        std::unique_ptr<Scene> upLoadedScene = std::make_unique<Scene>();
        auto &rootNode = upLoadedScene->GetRoot();

        tinyxml2::XMLDocument document;
        std::string path = m_resourceDir + std::string("scenes/") + envFilename;
        tinyxml2::XMLError result = document.LoadFile(path.c_str());
        if (result != tinyxml2::XML_SUCCESS)
        {
            return {};
        }

        tinyxml2::XMLNode* pSceneRoot = document.FirstChildElement("Scene");
        if (!pSceneRoot)
        {
            return {};
        }

        for (tinyxml2::XMLElement* pGameObject = pSceneRoot->FirstChildElement("GameObject"); pGameObject != nullptr; pGameObject = pGameObject->NextSiblingElement("GameObject"))
        {
            std::shared_ptr<Scene::Node> spChildNode = std::make_unique<Scene::Node>(upLoadedScene.get(), gameObjectId++);
            uint32_t nodeFlags = 0;

            // If we have a transform, we need to read it in and update the Scene node for this object
            tinyxml2::XMLElement *pTransform = pGameObject->FirstChildElement("Transform");
            {
                Transform nodeTransform;
                // Load up the transform information
                tinyxml2::XMLElement *pPosition = pTransform->FirstChildElement("Position");
                if (pPosition)
                {
                    nodeTransform.SetPosition(TinyXmlUtils::ParseVector3(*pPosition));
                }
                tinyxml2::XMLElement *pRotation = pTransform->FirstChildElement("Rotation");
                if (pRotation)
                {

                    Vector3 rotation = TinyXmlUtils::ParseVector3(*pRotation);
                    nodeTransform.SetRotation(Quaternion::RotationYawPitchRoll(rotation.z * F_PI / 180.0f, rotation.y * F_PI / 180.0f, rotation.x * F_PI / 180.0f));
                }
                tinyxml2::XMLElement *pScale = pTransform->FirstChildElement("Scale");
                if (pScale)
                {
                    nodeTransform.SetScale(TinyXmlUtils::ParseVector3(*pScale));
                }
                nodeTransform.UpdateWorld();
                spChildNode->SetTransform(nodeTransform);
            }

            // Look to see if we have any geometry
            tinyxml2::XMLElement* pGeometry = pGameObject->FirstChildElement("Geometry");
            if (pGeometry)
            {
                std::string geometryType = pGeometry->Attribute("type");
                if (geometryType.empty())
                {
                    continue;
                }

                if (geometryType == "tri-mesh")
                {
                    std::unique_ptr<TriMesh> upTriMesh = std::make_unique<TriMesh>();
                    // We need to iterate over the triangles
                    for (tinyxml2::XMLElement* pTriangle = pGeometry->FirstChildElement("Triangle"); pTriangle != nullptr; pTriangle = pGeometry->NextSiblingElement("Triangle"))
                    {
                        std::vector<Geometry::GeometryVertex> vertices;
                        // Load up the vertices
                        for (tinyxml2::XMLElement* pVertex = pTriangle->FirstChildElement("Vertex"); pVertex != nullptr; pVertex = pTriangle->NextSiblingElement("Vertex"))
                        {
                            Geometry::GeometryVertex vertex;
                            vertex.m_position = TinyXmlUtils::ParseVector3(*pVertex);
                            vertex.m_uv = Vector2(0.0f, 0.0f);
                            vertex.m_normal = Vector3(0.0f, 1.0f, 0.0f);
                            vertex.m_tangent = Vector3(0.0f, 1.0f, 0.0f);
                            vertices.push_back(vertex);
                        }
                        assert(vertices.size() == 3);
                        upTriMesh->AddTriangle(vertices[0], vertices[1], vertices[2]);
                        spChildNode->SetGeometry(upTriMesh.get());
                        m_upRenderer;
                        m_geometryCache.push_back(std::move(upTriMesh));
                    }
                }

                else if (geometryType == "obj-mesh")
                {
                    std::unique_ptr<ObjMesh> upObjMesh = std::make_unique<ObjMesh>();
                    std::string modelName = pGeometry->Attribute("name");
                    if (modelName.empty())
                    {
                        continue;
                    }
                    upObjMesh->Load("./assets/models/", modelName);
                    spChildNode->SetGeometry(upObjMesh.get());
                    // Add to cache so it never is changed
                    m_geometryCache.push_back(std::move(upObjMesh));
                }
                nodeFlags |= Scene::Node::NodeFlags::HasGeometry;
            }

            spChildNode->SetNodeFlags(nodeFlags);
            rootNode->AddChild(spChildNode);
        }
        return std::move(upLoadedScene);
    }

    int Game::Run()
    {
        // Frame counting averages
        const float targetFrameRateInSec = 1.0f / 50.0f;
        const int numFrameRateAvgCount = 10;
        float prevFrameTimings[numFrameRateAvgCount];

        float timeRunningTotal = 0.0f;
        int currentFrameIndex = 0;

        // These are all the fixed updates
        FixedUpdate physicsUpdate{static_cast<const float>(targetFrameRateInSec)};
        FixedUpdate controllerConnectionUpdate{2.0f};

        // Start controllers as connected if already when game starts
        m_upGameTimer->Reset();

        int frameRate = 0;

        // Ensure we are showing the game window
        m_pGameWindow->ShowGameWindow();

        uint32_t currentSceneIndex = 0;
        // We want to initialize the cloud scene.
        LoadSceneFarlor("Cornell.xml");



        // Initialize physics stuff
        std::unique_ptr<btDefaultCollisionConfiguration> upCollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
        std::unique_ptr<btCollisionDispatcher> upDispatcher = std::make_unique<btCollisionDispatcher>(upCollisionConfiguration.get());
        std::unique_ptr <btBroadphaseInterface> upOverlappingPairCache = std::make_unique<btDbvtBroadphase>();
        std::unique_ptr<btSequentialImpulseConstraintSolver> upSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
        std::unique_ptr<btDiscreteDynamicsWorld> upDynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
            upDispatcher.get(), upOverlappingPairCache.get(), upSolver.get(), upCollisionConfiguration.get()
        );
        upDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));

        btAlignedObjectArray<btCollisionShape*> collisionShapes;

        // Creating ground plane
        {
            btCollisionShape* pGroundShape = new btBoxShape(btVector3(50.0f, 50.0f, 50.0f));
            collisionShapes.push_back(pGroundShape);

            btTransform groundTransform;
            groundTransform.setIdentity();
            groundTransform.setOrigin(btVector3(0.0f, -56.0f, 0.0f));

            btScalar mass(0.0f);

            bool isDynamic = (mass != 0.0f);
            btVector3 localInertia(0.0f, 0.0f, 0.0f);
            if (isDynamic)
            {
                pGroundShape->calculateLocalInertia(mass, localInertia);
            }

            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, nullptr, pGroundShape, localInertia);
            btRigidBody* pBody = new btRigidBody(rbInfo);
            upDynamicsWorld->addRigidBody(pBody);
        }

        // Creating Sphere
        {
            btCollisionShape* pSphereShape = new btSphereShape(1.0f);
            collisionShapes.push_back(pSphereShape);

            btTransform startingTransform;
            startingTransform.setIdentity();

            btScalar sphereMass(1.0f);

            bool isDynamic = (sphereMass != 0.0f);
            btVector3 localInertia(0.0f, 0.0f, 0.0f);
            if (isDynamic)
            {
                pSphereShape->calculateLocalInertia(sphereMass, localInertia);
            }
            startingTransform.setOrigin(btVector3(2.0f, 10.0f, 0.0f));

            btRigidBody::btRigidBodyConstructionInfo rbInfo(sphereMass, nullptr, pSphereShape, localInertia);
            btRigidBody* pBody = new btRigidBody(rbInfo);
            upDynamicsWorld->addRigidBody(pBody);
        }

        for (int simStepIdx = 0; simStepIdx < 150; simStepIdx++)
        {
            upDynamicsWorld->stepSimulation(1.0f / 60.0f, 10);

            for (int objIdx = upDynamicsWorld->getNumCollisionObjects() - 1; objIdx >= 0; objIdx--)
            {
                btCollisionObject* pObj = upDynamicsWorld->getCollisionObjectArray()[objIdx];
                btRigidBody* pBody = btRigidBody::upcast(pObj);
                btTransform trans;
                if (pBody && pBody->getMotionState())
                {
                    pBody->getMotionState()->getWorldTransform(trans);
                }
                else
                {
                    trans = pObj->getWorldTransform();
                }
                printf("World Pos Object %d = %f, %f, %f\n", objIdx, trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
            }
        }

        while(m_running)
        {
            m_upGameTimer->Tick();
            m_upInputStateManager->Tick();

            float deltaTime = m_upGameTimer->DeltaTimeInSeconds();

            // Frame timing stuff
            prevFrameTimings[currentFrameIndex] = deltaTime;
            timeRunningTotal += deltaTime;
            currentFrameIndex++;

            if (currentFrameIndex >= numFrameRateAvgCount)
            {
                std::stringstream titleTextStream;
                frameRate = static_cast<int>(1.0f / (timeRunningTotal / numFrameRateAvgCount));
                titleTextStream << "Farlor Game: " << frameRate;
                currentFrameIndex = 0;
                timeRunningTotal = 0.0;
                m_pGameWindow->SetWindowTitle(titleTextStream.str());
            }

            physicsUpdate.AccumulateTime(deltaTime);
            controllerConnectionUpdate.AccumulateTime(deltaTime);

            // Process messages
            m_pGameWindow->ProcessEvent();

            // At this point, we can only read the input
            const InputState& latestInputState = m_upInputStateManager->GetLatestInputState();

            if (latestInputState.m_keyboardButtonStates[KeyboardButtons::Escape].endedDown)
            {
                m_running = false;
            }

            if (latestInputState.m_keyboardButtonStates[KeyboardButtons::UpArrow].endedDown)
            {
                const uint32_t NumScenes = 2;
                currentSceneIndex++;
                currentSceneIndex = currentSceneIndex % NumScenes;

                switch(currentSceneIndex)
                {
                    case 0:
                    {
                        LoadSceneFarlor(std::string("Cornell.xml"));
                    } break;

                    default:
                    {
                    } break;
                };
            }

            Camera* pCurrentCam = m_cameraManager.GetCurrentCamera();
            if (pCurrentCam)
            {
                pCurrentCam->Update(deltaTime, latestInputState);
            }

            static float totalTime = 0.0f;
            totalTime += deltaTime;
            //std::cout << "Total time: " << totalTime << std::endl;

            m_upRenderer->RenderFrame(deltaTime, totalTime);
        }

        m_pGameWindow->Shutdown();
        return 0;
    }
}
