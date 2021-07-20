#include "PBRTParser.h"

#include "StringUtil.h"

#include <FMath\Vector2.h>
#include <FMath\Vector3.h>

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

#include <assert.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace Farlor
{
    PBRTParser::PBRTParser()

        : m_upCurrentWorld{ nullptr }
        , m_pCurrentParentNode{ nullptr }
        , m_inFile{}
        , m_sceneRootDir{""}
        , m_specifyWorld{ false }
        , m_currentWorldState{}
    {
    }

    std::unique_ptr<World> PBRTParser::ParseScene(const std::string& sceneFilename)
    {
        std::cout << "Parsing file: " << sceneFilename << std::endl;
        m_sceneRootDir = Utility::StringUtil::RemoveFilenameFromPath(sceneFilename);

        std::cout << "Scene root dir: " << m_sceneRootDir << std::endl;

        BeginParsing(sceneFilename);

        std::string line{};
        while (std::getline(m_inFile, line))
        {
            ParseLine(line);
        }

        return std::move(m_upCurrentWorld);
    }

    // Called when parsing begins. This sets up required state for parsing the scene
    void PBRTParser::BeginParsing(const std::string& sceneFilename)
    {
        m_upCurrentWorld = std::make_unique<World>();
        m_pCurrentParentNode = m_upCurrentWorld->GetRootNode();

        m_inFile.open(sceneFilename);
        if (!m_inFile)
        {
            std::cout << "Failed to open file for parsing: " << sceneFilename << std::endl;
        }
    }

    // Called when parsing begins. This allows shutting down of state after parsing
    void PBRTParser::EndParsing()
    {
        if (m_worldStateStack.size())
        {
            std::cout << "Error: World Stack still not empty" << std::endl;
        }
    }

    // Actual Parsing Functions
    void PBRTParser::ParseLine(std::string& line)
    {
        Farlor::Utility::StringUtil::Trim(line);
        std::string command = Farlor::Utility::StringUtil::GetAndRemoveFirstWord(line);

        // If no command!
        if (command.empty())
        {
            return;
        }

        // Simply ignore comments
        if (command.at(0) == '#')
        {
            // Comment Line
            return;
        }

        else if (m_specifyWorld)
        {
            if (command == std::string("WorldEnd"))
            {
                m_specifyWorld = false;
            }
            else if (command == std::string("NamedMaterial"))
            {
                std::string param = ParseParam(line);
                //std::cout << "Need to set named material: " << param << std::endl;
                //m_upPBRTScene->SetActiveMaterial(param);
            }
            else if (command == std::string("Shape"))
            {
                std::unique_ptr<Mesh> upMesh = ParseShape(line);
                if (upMesh)
                {
                    uint32_t meshId = m_upCurrentWorld->AddMesh(std::move(upMesh));
                    if (m_currentWorldState.pAreaLight)
                    {
                        m_currentWorldState.pAreaLight->SetMesh(meshId);
                    }

                    std::unique_ptr<SceneGraphNode> upNewNode = std::make_unique<SceneGraphNode>(meshId);
                    m_pCurrentParentNode->AddChild(std::move(upNewNode));
                }
            }
            else if (command == std::string("AttributeBegin"))
            {
                PushGraphicsState();
            }
            else if (command == std::string("AttributeEnd"))
            {
                PopGraphicsState();
            }
            else
            {
                std::cout << "Unsupported Command: " << command << std::endl;
            }
        }

        // We are not specifying the world currently!!
        else
        {
            if (command == std::string("WorldBegin"))
            {
                m_specifyWorld = true;
            }
            else if (command == std::string("Camera"))
            {
                while (!line.empty())
                {
                    std::string param = ParseParam(line);
                    if (param.empty())
                    {
                        break;
                    }
                    //std::cout << "Param:" << param << std::endl;
                }

                while (!line.empty())
                {
                    std::string argument = ParseArgument(line);
                    if (argument.empty())
                    {
                        break;
                    }
                }

            }
            else
            {
                std::cout << "Unsupported Command: " << command << std::endl;
            }
        }
    }

    std::unique_ptr<Mesh> PBRTParser::ParseShape(std::string& line)
    {
        std::string meshType = ParseParam(line);

        std::unique_ptr<Mesh> upMesh = nullptr;

        if (meshType == "plymesh")
        {
            upMesh = LoadPlymesh(line);
        }
        else if (meshType == "trianglemesh")
        {
            upMesh = LoadTriMesh(line);
        }
        else
        {
            std::cout << "Unsupported command" << std::endl;
        }

        return upMesh;
    }

    std::unique_ptr<Mesh> PBRTParser::LoadTriMesh(std::string& line)
    {
        std::vector<Vector3> positions{};
        std::vector<Vector3> normals{};
        std::vector<Vector2> uvs{};
        std::vector<uint32_t> indicies{};

        uint32_t meshFlags = 0;

        while (!line.empty())
        {
            // We parse the type
            std::string param = ParseParam(line);
            // Then we parse the argument and fill the structure
            std::string argument = ParseArgument(line);

            // We are grabbing numerical data, so lets put the argument in a string stream so we can put it directly into the structures we want.
            std::stringstream argumentStream{ argument };

            if (param == "integer indices")
            {
                uint32_t index;
                while (argumentStream >> index)
                {
                    indicies.push_back(index);
                }
            }
            else if (param == "point P")
            {
                meshFlags |= static_cast<uint32_t>(Mesh::VertexFlags::Position);
                float x = 0.0f, y = 0.0f, z = 0.0f;
                while (argumentStream >> x >> y >> z)
                {
                    positions.emplace_back(x, y, z);
                }
            }
            else if (param == "normal N")
            {
                meshFlags |= static_cast<uint32_t>(Mesh::VertexFlags::Normal);
                float x = 0.0f, y = 0.0f, z = 0.0f;
                while (argumentStream >> x >> y >> z)
                {
                    normals.emplace_back(x, y, z);
                }
            }
            else if (param == "float uv")
            {
                meshFlags |= static_cast<uint32_t>(Mesh::VertexFlags::UV);
                float u = 0.0f, v = 0.0f;
                while (argumentStream >> u >> v)
                {
                    uvs.emplace_back(u, v);
                }
            }
            else
            {
            }
        }

        std::vector<Mesh::Vertex> vertices{};
        for (uint32_t i = 0; i < positions.size(); ++i)
        {
            Mesh::Vertex vertex;
            vertex.m_position = positions[i];
            if (meshFlags & static_cast<uint32_t>(Mesh::VertexFlags::Normal))
            {
                vertex.m_normal = normals[i];
            }

            if (meshFlags & static_cast<uint32_t>(Mesh::VertexFlags::UV))
            {
                vertex.m_uv = uvs[i];
            }
            vertices.push_back(vertex);
        }

        return std::make_unique<Mesh>(std::move(vertices), std::move(indicies), meshFlags);
    }

    std::unique_ptr<Mesh> PBRTParser::LoadPlymesh(std::string& line)
    {
        auto firstPart = ParseParam(line);
        if (firstPart != "string filename")
        {
            return nullptr;
        }

        auto filenameParam = ParseArgument(line);
        auto filename = ParseParam(filenameParam);
        std::string plyFilepath = m_sceneRootDir + filename;

        Assimp::Importer assetImporter{};
        const aiScene* pScene = assetImporter.ReadFile(plyFilepath, aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);
        if (!pScene)
        {
            std::cout << "Failed to load scene" << std::endl;
            return nullptr;
        }

        // This scene should only have ONE mesh in it, at least in the pbrt format
        assert(pScene->mNumMeshes == 1);
        aiMesh* pMesh = pScene->mMeshes[0];

        std::vector<Mesh::Vertex> vertices{};
        std::vector<uint32_t> indicies{};

        for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
        {
            const aiFace& face = pMesh->mFaces[i];

            for (int j = 0; j<3; j++)
            {
                Mesh::Vertex vertex;
                aiVector3D uv = pMesh->mTextureCoords[0][face.mIndices[j]];
                vertex.m_uv = Farlor::Vector2(uv.x, uv.y);

                aiVector3D normal = pMesh->mNormals[face.mIndices[j]];
                vertex.m_normal = Farlor::Vector3(normal.x, normal.y, normal.z);

                aiVector3D pos = pMesh->mVertices[face.mIndices[j]];
                vertex.m_position = Farlor::Vector3(pos.x, pos.y, pos.z);
                vertices.push_back(vertex);
                indicies.push_back(face.mIndices[j]);
            }
        }

        return std::make_unique<Mesh>(std::move(vertices), std::move(indicies), static_cast<uint32_t>(Mesh::VertexFlags::Position) | static_cast<uint32_t>(Mesh::VertexFlags::Normal) | static_cast<uint32_t>(Mesh::VertexFlags::UV));
    }

    void PBRTParser::ParseMakeNamedMaterial(std::string& line)
    {
    }

    void PBRTParser::ParseTransform(std::string& line)
    {
    }

    std::string PBRTParser::ParseParam(std::string& line)
    {
        size_t firstQ = line.find('\"');
        size_t secondQ = line.find('\"', firstQ+1);

        if (firstQ == std::string::npos || secondQ == std::string::npos)
        {
            return{};
        }

        std::string param = line.substr(firstQ + 1, secondQ-1);
        Farlor::Utility::StringUtil::Trim(param);

        line = line.substr(secondQ + 1, std::string::npos);
        Farlor::Utility::StringUtil::Trim(line);
        return param;
    }
    
    std::string PBRTParser::ParseArgument(std::string& line)
    {
        size_t firstQ = line.find('[');
        size_t secondQ = line.find(']');

        if (firstQ == std::string::npos || secondQ == std::string::npos)
        {
            return{};
        }

        std::string argList = line.substr(firstQ + 1, secondQ - 1);
        Farlor::Utility::StringUtil::Trim(argList);

        line = line.substr(secondQ + 1, std::string::npos);
        Farlor::Utility::StringUtil::Trim(line);
        return argList;
    }

    void PBRTParser::PushGraphicsState()
    {
        WorldState newState = m_currentWorldState;
        m_worldStateStack.push(m_currentWorldState);
        m_currentWorldState = newState;
    }

    void PBRTParser::PopGraphicsState()
    {
        m_currentWorldState = m_worldStateStack.top();
        m_worldStateStack.pop();
    }
}