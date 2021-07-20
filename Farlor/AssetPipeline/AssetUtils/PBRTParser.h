#pragma once

#include <World.h>

#include <fstream>
#include <memory>
#include <stack>
#include <string>

namespace Farlor
{
    // This class is responsible for parsing a pbrt scene file and constructing a PBRTScene that can be traversed.
    class PBRTParser
    {
        struct WorldState
        {
            //Material* pCurrentMaterial;
            AreaLight* pAreaLight;
        };

    public:
        explicit PBRTParser();

        std::unique_ptr<World> ParseScene(const std::string& sceneFilename);

    private:
        // Called when parsing begins. This sets up required state for parsing the scene
        void BeginParsing(const std::string& sceneFilename);
        // Called when parsing begins. This allows shutting down of state after parsing
        void EndParsing();

        void PushGraphicsState();
        void PopGraphicsState();

        // Actual Parsing Functions
        void ParseLine(std::string& line);
        std::unique_ptr<Mesh> ParseShape(std::string& line);
        void ParseMakeNamedMaterial(std::string& line);
        void ParseTransform(std::string& line);
        
        std::unique_ptr<Mesh> LoadPlymesh(std::string& line);
        std::unique_ptr<Mesh> LoadTriMesh(std::string& line);

        std::string ParseParam(std::string& line);
        std::string ParseArgument(std::string& line);

    private:
        std::unique_ptr<World> m_upCurrentWorld;
        SceneGraphNode* m_pCurrentParentNode;
        std::ifstream m_inFile;
        std::string m_sceneRootDir;

        // Parsing State Management
        bool m_specifyWorld;

        std::stack<WorldState> m_worldStateStack;
        WorldState m_currentWorldState;
    };
}