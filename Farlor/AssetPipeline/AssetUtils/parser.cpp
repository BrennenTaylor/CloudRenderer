
/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include "parser.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace pbrt
{
    // Static paser location.
    // Why is this globL
    Loc* g_g_pParserLoc;

    // Why is this static?
    char DecodeEscapedCharacter(int ch)
    {
        switch (ch)
        {
        case EOF:
            assert(false);
            exit(1);
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        default:
            assert(false, "Unexpected character");
            exit(1);
        }
        // TODO: Error
        return 0;
    }

    // Factory methods
    std::unique_ptr<Tokenizer> Tokenizer::CreateFromFile(const std::string& filename, std::function<void(const char *)> errorCallback)
    {
        // Lets use filesystem.
        FILE* pFile = fopen(filename.c_str(), "r");
        if (!pFile)
        {
            std::stringstream tempSS;
            tempSS << filename << ": " << strerror(errno);
            errorCallback(tempSS.str().c_str());
            return nullptr;
        }

        std::string str;
        int ch;
        while ((ch = fgetc(pFile)) != EOF)
        {
            str.push_back(char(ch));
        }        
        fclose(pFile);

        return std::make_unique<Tokenizer>(std::move(str), std::move(errorCallback));
    }

    std::unique_ptr<Tokenizer> Tokenizer::CreateFromString(const std::string str, std::function<void(const char *)> errorCallback)
    {
        return std::make_unique<Tokenizer>(str, std::move(errorCallback));
    }


    // Actual tokenizer methods
    Tokenizer::Tokenizer(std::string str, std::function<void(const char *)> errorCallback)
        : m_currentLoc("")
        , m_errorCallback(std::move(errorCallback))
        , m_contents(std::move(str))
    {
        m_pCurrentPos = m_contents.data();
        m_pEndPos = m_pCurrentPos + m_contents.size();
    }

    Tokenizer::~Tokenizer()
    {
    }

    string_view Tokenizer::NextToken()
    {
        while (true)
        {
            const char* pTokenStart = m_pCurrentPos;
            int ch = GetNextChar();
            if (ch == EOF)
            {
                return {nullptr, 0};
            }
            else if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
            {
                // Keep on going, we just skip this stuff
            }
            // * ..... * is a token
            else if (ch == '"')
            {
                // Scan to closing quote
                bool haveEscaped = false;
                while ((ch = GetNextChar()) != '"')
                {
                    if (ch == EOF)
                    {
                        m_errorCallback("premature EOF");
                        return {};
                    }
                    else if (ch == '\n')
                    {
                        m_errorCallback("unterminated string");
                        return {};
                    } 
                    else if (ch == '\\')
                    {
                        haveEscaped = true;
                        // Grab the next character
                        if ((ch = GetNextChar()) == EOF)
                        {
                            m_errorCallback("premature EOF");
                            return {};
                        }
                    }
                }

                if (!haveEscaped)
                {
                    // Get Next Char updates m_pCurrentPos
                    return { pTokenStart, size_t(m_pCurrentPos - pTokenStart) };
                }
                else
                {
                    m_processedEscaped.clear();
                    for (const char* pChar = pTokenStart; pChar < m_pCurrentPos; ++pChar)
                    {
                        if (*pChar != '\\')
                        {
                            m_processedEscaped.push_back(*pChar);
                        }
                        else
                        {
                            ++pChar;
                            // TODO: assert?
                            // If this fails, we have an escape character that is invalid
                            assert(pChar < m_pCurrentPos);
                            m_processedEscaped.push_back(DecodeEscapedCharacter(*pChar));
                        }
                    }
                    return {m_processedEscaped.data(), m_processedEscaped.size()};
                }
            }
            // [ and ] are tokens
            else if (ch == '[' || ch == ']')
            {
                return {pTokenStart, size_t(1)};
            }
            // We want to not have comments
            else if (ch == '#')
            {
                // comment: scan to EOL (or EOF)
                while ((ch = GetNextChar()) != EOF)
                {
                    // Go to the end of the line
                    if (ch == '\n' || ch == '\r')
                    {
                        PutbackChar();
                        break;
                    }
                }

                return {pTokenStart, size_t(m_pCurrentPos - pTokenStart)};
            }

            // This is a normal token. Special characters and spaces mark end of the token
            else
            {
                // Regular statement or numeric token; scan until we hit a
                // space, opening quote, or bracket.
                while ((ch = GetNextChar()) != EOF)
                {
                    if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' ||
                        ch == '"' || ch == '[' || ch == ']')
                    {
                        PutbackChar();
                        break;
                    }
                }
                return {pTokenStart, size_t(m_pCurrentPos - pTokenStart)};
            }
        }
    }

    float ParseNumber(string_view str)
    {
        // Fast path for a single digit
        if (str.size() == 1)
        {
            if (!(str[0] >= '0' && str[0] <= '9'))
            {
                printf("\"%c\": expected a number", str[0]);
                exit(1);
            }
            return str[0] - '0';
        }

        // Copy to a buffer so we can NUL-terminate it, as strto[idf]() expect.
        char buf[64];
        char* bufp = buf;
        std::unique_ptr<char[]> allocBuf;
        if (str.size() + 1 >= sizeof(buf))
        {
            // This should be very unusual, but is necessary in case we get a
            // goofball number with lots of leading zeros, for example.
            allocBuf.reset(new char[str.size() + 1]);
            bufp = allocBuf.get();
        }

        std::copy(str.begin(), str.end(), bufp);
        bufp[str.size()] = '\0';

        // Can we just use strtol?
        auto IsInteger = [](string_view str)
        {
            for (char ch : str)
            {
                if (!(ch >= '0' && ch <= '9'))
                {
                    return false;
                }
            }
            return true;
        };

        char *endptr = nullptr;
        double val;
        if (IsInteger(str))
        {
            val = float(strtol(bufp, &endptr, 10));
        }
        else
        {
            val = strtof(bufp, &endptr);
        }

        if (val == 0 && endptr == bufp)
        {
            printf("%s: expected a number", str.toString().c_str());
            exit(1);
        }

        return val;
    }

    bool IsQuotedString(string_view str)
    {
        return str.size() >= 2 && str[0] == '"' && str.back() == '"';
    }

    static string_view dequoteString(string_view str)
    {
        if (!IsQuotedString(str))
        {
            printf("\"%s\": expected quoted string", str.toString().c_str());
            exit(1);
        }

        str.remove_prefix(1);
        str.remove_suffix(1);
        return str;
    }

    // ????
    struct ParamListItem
    {
        std::string name;
        std::vector<float> numericalValues;
        std::vector<std::string> stringValues;
        size_t size = 0;
        bool isString = false;
    };

    // TODO: Make this enum class
    constexpr int TokenOptional = 0;
    constexpr int TokenRequired = 1;

    // TODO: Make this enum class
    enum
    {
        PARAM_TYPE_INT,
        PARAM_TYPE_BOOL,
        PARAM_TYPE_FLOAT,
        PARAM_TYPE_POINT2,
        PARAM_TYPE_VECTOR2,
        PARAM_TYPE_POINT3,
        PARAM_TYPE_VECTOR3,
        PARAM_TYPE_NORMAL,
        PARAM_TYPE_RGB,
        PARAM_TYPE_XYZ,
        PARAM_TYPE_BLACKBODY,
        PARAM_TYPE_SPECTRUM,
        PARAM_TYPE_STRING,
        PARAM_TYPE_TEXTURE
    };

    bool LookupType(const std::string& decl, int* type, std::string& sname)
    {
        *type = 0;
        // Skip leading space
        auto SkipSpace = [&decl](std::string::const_iterator iter) {
            while (iter != decl.end() && (*iter == ' ' || *iter == '\t'))
            {
                ++iter;
            }
            return iter;
        };

        // Skip to the next whitespace character (or the end of the string).
        auto SkipToSpace = [&decl](std::string::const_iterator iter) {
            while (iter != decl.end() && *iter != ' ' && *iter != '\t')
            {
                ++iter;
            }
            return iter;
        };

        auto typeBegin = SkipSpace(decl.begin());
        if (typeBegin == decl.end())
        {
            printf("Parameter \"%s\" doesn't have a type declaration?!", decl.c_str());
            return false;
        }

        // Find end of type declaration
        auto typeEnd = SkipToSpace(typeBegin);

        string_view typeStr(&(*typeBegin), size_t(typeEnd - typeBegin));
        if (typeStr == "float")
            *type = PARAM_TYPE_FLOAT;
        else if (typeStr == "integer")
            *type = PARAM_TYPE_INT;
        else if (typeStr == "bool")
            *type = PARAM_TYPE_BOOL;
        else if (typeStr == "point2")
            *type = PARAM_TYPE_POINT2;
        else if (typeStr == "vector2")
            *type = PARAM_TYPE_VECTOR2;
        else if (typeStr == "point3")
            *type = PARAM_TYPE_POINT3;
        else if (typeStr == "vector3")
            *type = PARAM_TYPE_VECTOR3;
        else if (typeStr == "point")
            *type = PARAM_TYPE_POINT3;
        else if (typeStr == "vector")
            *type = PARAM_TYPE_VECTOR3;
        else if (typeStr == "normal")
            *type = PARAM_TYPE_NORMAL;
        else if (typeStr == "string")
            *type = PARAM_TYPE_STRING;
        else if (typeStr == "texture")
            *type = PARAM_TYPE_TEXTURE;
        else if (typeStr == "color")
            *type = PARAM_TYPE_RGB;
        else if (typeStr == "rgb")
            *type = PARAM_TYPE_RGB;
        else if (typeStr == "xyz")
            *type = PARAM_TYPE_XYZ;
        else if (typeStr == "blackbody")
            *type = PARAM_TYPE_BLACKBODY;
        else if (typeStr == "spectrum")
            *type = PARAM_TYPE_SPECTRUM;
        else {
            printf("Unable to decode type from \"%s\"", decl.c_str());
            return false;
        }

        auto nameBegin = SkipSpace(typeEnd);
        if (nameBegin == decl.end())
        {
            printf("Unable to find parameter name from \"%s\"", decl.c_str());
            return false;
        }
        auto nameEnd = SkipToSpace(nameBegin);
        sname = std::string(nameBegin, nameEnd);

        return true;
    }

    static const char* ParamTypeToName(int type)
    {
        switch (type) {
        case PARAM_TYPE_INT:
            return "int";
        case PARAM_TYPE_BOOL:
            return "bool";
        case PARAM_TYPE_FLOAT:
            return "float";
        case PARAM_TYPE_POINT2:
            return "point2";
        case PARAM_TYPE_VECTOR2:
            return "vector2";
        case PARAM_TYPE_POINT3:
            return "point3";
        case PARAM_TYPE_VECTOR3:
            return "vector3";
        case PARAM_TYPE_NORMAL:
            return "normal";
        case PARAM_TYPE_RGB:
            return "rgb/color";
        case PARAM_TYPE_XYZ:
            return "xyz";
        case PARAM_TYPE_BLACKBODY:
            return "blackbody";
        case PARAM_TYPE_SPECTRUM:
            return "spectrum";
        case PARAM_TYPE_STRING:
            return "string";
        case PARAM_TYPE_TEXTURE:
            return "texture";
        default:
            printf("Error in paramTypeToName");
            return nullptr;
        }
    }

    // TODO: How should we refactor handling parameters
    
    /*
    static void AddParam(ParamSet& ps, const ParamListItem &item)
    {
        int type;
        std::string name;
        if (lookupType(item.name, &type, name))
        {
            if (type == PARAM_TYPE_TEXTURE || type == PARAM_TYPE_STRING || type == PARAM_TYPE_BOOL)
            {
                if (!item.stringValues)
                {
                    Error(
                        "Expected string parameter value for parameter "
                        "\"%s\" with type \"%s\". Ignoring.",
                        name.c_str(), paramTypeToName(type));
                    return;
                }
            }
            else if (type != PARAM_TYPE_SPECTRUM)
            {
                if (item.stringValues)
                {
                    printf("Expected numeric parameter value for parameter " "\"%s\" with type \"%s\".  Ignoring.", name.c_str(), paramTypeToName(type));
                    return;
                }
            }

            int nItems = item.size;
            if (type == PARAM_TYPE_INT)
            {
                // parser doesn't handle ints, so convert from doubles here....
                int nAlloc = nItems;
                std::unique_ptr<int[]> idata(new int[nAlloc]);
                for (int j = 0; j < nAlloc; ++j)
                {
                    idata[j] = int(item.doubleValues[j]);
                }
                ps.AddInt(name, std::move(idata), nItems);
            }
            else if (type == PARAM_TYPE_BOOL)
            {
                // strings -> bools
                int nAlloc = item.size;
                std::unique_ptr<bool[]> bdata(new bool[nAlloc]);
                for (int j = 0; j < nAlloc; ++j)
                {
                    std::string s(item.stringValues[j]);
                    if (s == "true")
                    {
                        bdata[j] = true;
                    }
                    else if (s == "false")
                    {
                        bdata[j] = false;
                    }
                    else
                    {
                        printf("Value \"%s\" unknown for Boolean parameter \"%s\"." "Using \"false\".", s.c_str(), item.name.c_str());
                        bdata[j] = false;
                    }
                }
                ps.AddBool(name, std::move(bdata), nItems);
            }
            else if (type == PARAM_TYPE_FLOAT)
            {
                std::unique_ptr<Float[]> floats(new Float[nItems]);
                for (int i = 0; i < nItems; ++i) floats[i] = item.doubleValues[i];
                ps.AddFloat(name, std::move(floats), nItems);
            }
            else if (type == PARAM_TYPE_POINT2)
            {
                if ((nItems % 2) != 0)
                {
                    Warning("Excess values given with point2 parameter \"%s\". " "Ignoring last one of them.", item.name.c_str());
                }
                std::unique_ptr<Point2f[]> pts(new Point2f[nItems / 2]);
                for (int i = 0; i < nItems / 2; ++i) {
                    pts[i].x = item.doubleValues[2 * i];
                    pts[i].y = item.doubleValues[2 * i + 1];
                }
                ps.AddPoint2f(name, std::move(pts), nItems / 2);
            } else if (type == PARAM_TYPE_VECTOR2) {
                if ((nItems % 2) != 0)
                    Warning(
                        "Excess values given with vector2 parameter \"%s\". "
                        "Ignoring last one of them.",
                        item.name.c_str());
                std::unique_ptr<Vector2f[]> vecs(new Vector2f[nItems / 2]);
                for (int i = 0; i < nItems / 2; ++i) {
                    vecs[i].x = item.doubleValues[2 * i];
                    vecs[i].y = item.doubleValues[2 * i + 1];
                }
                ps.AddVector2f(name, std::move(vecs), nItems / 2);
            } else if (type == PARAM_TYPE_POINT3) {
                if ((nItems % 3) != 0)
                    Warning(
                        "Excess values given with point3 parameter \"%s\". "
                        "Ignoring last %d of them.",
                        item.name.c_str(), nItems % 3);
                std::unique_ptr<Point3f[]> pts(new Point3f[nItems / 3]);
                for (int i = 0; i < nItems / 3; ++i) {
                    pts[i].x = item.doubleValues[3 * i];
                    pts[i].y = item.doubleValues[3 * i + 1];
                    pts[i].z = item.doubleValues[3 * i + 2];
                }
                ps.AddPoint3f(name, std::move(pts), nItems / 3);
            } else if (type == PARAM_TYPE_VECTOR3) {
                if ((nItems % 3) != 0)
                    Warning(
                        "Excess values given with vector3 parameter \"%s\". "
                        "Ignoring last %d of them.",
                        item.name.c_str(), nItems % 3);
                std::unique_ptr<Vector3f[]> vecs(new Vector3f[nItems / 3]);
                for (int j = 0; j < nItems / 3; ++j) {
                    vecs[j].x = item.doubleValues[3 * j];
                    vecs[j].y = item.doubleValues[3 * j + 1];
                    vecs[j].z = item.doubleValues[3 * j + 2];
                }
                ps.AddVector3f(name, std::move(vecs), nItems / 3);
            } else if (type == PARAM_TYPE_NORMAL) {
                if ((nItems % 3) != 0)
                    Warning(
                        "Excess values given with \"normal\" parameter \"%s\". "
                        "Ignoring last %d of them.",
                        item.name.c_str(), nItems % 3);
                std::unique_ptr<Normal3f[]> normals(new Normal3f[nItems / 3]);
                for (int j = 0; j < nItems / 3; ++j) {
                    normals[j].x = item.doubleValues[3 * j];
                    normals[j].y = item.doubleValues[3 * j + 1];
                    normals[j].z = item.doubleValues[3 * j + 2];
                }
                ps.AddNormal3f(name, std::move(normals), nItems / 3);
            } else if (type == PARAM_TYPE_RGB) {
                if ((nItems % 3) != 0) {
                    Warning(
                        "Excess RGB values given with parameter \"%s\". "
                        "Ignoring last %d of them",
                        item.name.c_str(), nItems % 3);
                    nItems -= nItems % 3;
                }
                std::unique_ptr<Float[]> floats(new Float[nItems]);
                for (int j = 0; j < nItems; ++j) floats[j] = item.doubleValues[j];
                ps.AddRGBSpectrum(name, std::move(floats), nItems);
            } else if (type == PARAM_TYPE_XYZ) {
                if ((nItems % 3) != 0) {
                    Warning(
                        "Excess XYZ values given with parameter \"%s\". "
                        "Ignoring last %d of them",
                        item.name.c_str(), nItems % 3);
                    nItems -= nItems % 3;
                }
                std::unique_ptr<Float[]> floats(new Float[nItems]);
                for (int j = 0; j < nItems; ++j) floats[j] = item.doubleValues[j];
                ps.AddXYZSpectrum(name, std::move(floats), nItems);
            } else if (type == PARAM_TYPE_BLACKBODY) {
                if ((nItems % 2) != 0) {
                    Warning(
                        "Excess value given with blackbody parameter \"%s\". "
                        "Ignoring extra one.",
                        item.name.c_str());
                    nItems -= nItems % 2;
                }
                std::unique_ptr<Float[]> floats(new Float[nItems]);
                for (int j = 0; j < nItems; ++j) floats[j] = item.doubleValues[j];
                ps.AddBlackbodySpectrum(name, std::move(floats), nItems);
            } else if (type == PARAM_TYPE_SPECTRUM) {
                if (item.stringValues) {
                    ps.AddSampledSpectrumFiles(name, item.stringValues, nItems);
                } else {
                    if ((nItems % 2) != 0) {
                        Warning(
                            "Non-even number of values given with sampled "
                            "spectrum "
                            "parameter \"%s\". Ignoring extra.",
                            item.name.c_str());
                        nItems -= nItems % 2;
                    }
                    std::unique_ptr<Float[]> floats(new Float[nItems]);
                    for (int j = 0; j < nItems; ++j)
                        floats[j] = item.doubleValues[j];
                    ps.AddSampledSpectrum(name, std::move(floats), nItems);
                }
            } else if (type == PARAM_TYPE_STRING) {
                std::unique_ptr<std::string[]> strings(new std::string[nItems]);
                for (int j = 0; j < nItems; ++j)
                    strings[j] = std::string(item.stringValues[j]);
                ps.AddString(name, std::move(strings), nItems);
            } else if (type == PARAM_TYPE_TEXTURE) {
                if (nItems == 1) {
                    std::string val(*item.stringValues);
                    ps.AddTexture(name, val);
                } else
                    Error(
                        "Only one string allowed for \"texture\" parameter "
                        "\"%s\"",
                        name.c_str());
            }
        } else
            Warning("Type of parameter \"%s\" is unknown", item.name.c_str());
    }
    */

    template <typename Next, typename Unget>
    ParamSet ParseParams(Next nextToken, Unget ungetToken, MemoryArena &arena)
    {
        // The goal is to store a set of parameters and return them
        ParamSet ps;
        while (true)
        {
            string_view decl = nextToken(TokenOptional);
            if (decl.empty())
            {
                return ps;
            }

            if (!IsQuotedString(decl))
            {
                ungetToken(decl);
                return ps;
            }

            ParamListItem item;
            item.name = DequoteString(decl).toString();

            size_t nAlloc = 0;

            auto addVal = [&](string_view val) {
                if (IsQuotedString(val))
                {
                    if (item.numericParams.size())
                    {
                        printf("mixed string and numeric parameters");
                        exit(1);
                    }
                    if (item.size == nAlloc)
                    {
                        nAlloc = std::max<size_t>(2 * item.size, 4);
                        const char** newData = arena.Alloc<const char *>(nAlloc);
                        std::copy(item.stringValues, item.stringValues + item.size, newData);
                        item.stringValues = newData;
                    }

                    val = dequoteString(val);
                    char *buf = arena.Alloc<char>(val.size() + 1);
                    memcpy(buf, val.data(), val.size());
                    buf[val.size()] = '\0';
                    item.stringValues[item.size++] = buf;
                }
                else
                {
                    if (item.stringValues)
                    {
                        printf("mixed string and numeric parameters");
                        exit(1);
                    }

                    if (item.size == nAlloc)
                    {
                        nAlloc = std::max<size_t>(2 * item.size, 4);
                        double *newData = arena.Alloc<double>(nAlloc);
                        std::copy(item.doubleValues, item.doubleValues + item.size, newData);
                        item.doubleValues = newData;
                    }
                    item.doubleValues[item.size++] = ParseNumber(val);
                }
            };

            string_view val = nextToken(TokenRequired);

            if (val == "[")
            {
                while (true)
                {
                    val = nextToken(TokenRequired);
                    if (val == "]")
                    {
                        break;
                    }
                    addVal(val);
                }
            }
            else
            {
                addVal(val);
            }

            AddParam(ps, item, spectrumType);
            arena.Reset();
        }

        return ps;
    }

    // What in the world is this?
    extern int catIndentCount;

    // Parsing Global Interface
    void PBRTParse(std::unique_ptr<Tokenizer> t)
    {
        // This is used to implement include parsing
        std::vector<std::unique_ptr<Tokenizer>> fileStack;

        fileStack.push_back(std::move(t));
        // This is global
        g_pParserLoc = &(fileStack.back()->m_currentLoc);

        bool tokenUngetRequired = false;
        std::string ungetTokenValue;

        // nextToken is a little helper function that handles the file stack,
        // returning the next token from the current file until reaching EOF,
        // at which point it switches to the next file (if any).
        std::function<string_view(int)> DoGetNextToken;
        DoGetNextToken = [&](int flags) -> string_view {
            
            // TODO: Figure out
            if (tokenUngetRequired)
            {
                tokenUngetRequired = false;
                return string_view(ungetTokenValue.data(), ungetTokenValue.size());
            }

            if (fileStack.empty())
            {
                // TODO: Should we change to enum?
                if (flags & TokenRequired)
                {
                    printf("premature EOF");
                    exit(1);
                }
                g_pParserLoc = nullptr;
                return {};
            }

            string_view nextToken = fileStack.back()->NextToken();

            if (nextToken.empty())
            {
                // We've reached EOF in the current file. Anything more to parse?
                fileStack.pop_back();
                if (!fileStack.empty())
                {
                    g_pParserLoc = &fileStack.back()->m_currentLoc;
                }
                return DoGetNextToken(flags);
            }
            else if (nextToken == "Include")
            {
                // Switch to the given file.
                // TODO: Use fulysystem apu
                
                /*std::string filename = toString(dequoteString(nextToken(TokenRequired)));
                filename = AbsolutePath(ResolveFilename(filename));*/
                
                auto tokError = [](const char* pMsg) {
                    printf("%s", pMsg);
                };

                std::unique_ptr<Tokenizer> includeFileTokenizer = Tokenizer::CreateFromFile(filename, tokError);
                if (includeFileTokenizer)
                {
                    fileStack.push_back(std::move(includeFileTokenizer));
                    g_pParserLoc = &fileStack.back()->m_currentLoc;
                }
                return DoGetNextToken(flags);
            }
            else if (nextToken[0] == '#')
            {
                // Swallow comments, unless --cat or --toply was given, in
                // which case they're printed to stdout.
                return DoGetNextToken(flags);
            }
            else
            {
                // Regular token; success.
                return nextToken;
            }
        };

        auto DoUngetToken = [&](string_view s) {
            assert(!tokenUngetRequired);
            ungetTokenValue = std::string(s.data(), s.size());
            tokenUngetRequired = true;
        };

        // Helper function for pbrt API entrypoints that take a single string
        // parameter and a ParamSet (e.g. pbrtShape()).
        auto BasicParamListEntrypoint = [&](std::function<void(const std::string& n, ParamSet p)> apiFunc) {
            string_view token = DoGetNextToken(TokenRequired);
            string_view dequoted = dequoteString(token);
            std::string n = dequoted.toString();
            ParamSet params = ParseParams(DoGetNextToken, DoUngetToken, arena);
            apiFunc(n, std::move(params));
        };

        auto SyntaxError = [&](string_view token) {
            printf("Unexpected token: %s", token.toString().c_str());
            exit(1);
        };

        while (true)
        {
            string_view nextToken = DoGetNextToken(TokenOptional);
            if (nextToken.empty())
            {
                break;
            }

            switch (nextToken[0])
            {
            case 'A':
                if (nextToken == "AttributeBegin")
                {
                    printf("pbrtAttributeBegin");
                }
                else if (nextToken == "AttributeEnd")
                {
                    printf("pbrtAttributeEnd();");
                }
                else if (nextToken == "ActiveTransform")
                {
                    string_view a = DoGetNextToken(TokenRequired);
                    if (a == "All")
                    {
                        printf("pbrtActiveTransformAll();");
                    }
                    else if (a == "EndTime")
                    {
                        printf("pbrtActiveTransformEndTime();");
                    }
                    else if (a == "StartTime")
                    {
                        printf("pbrtActiveTransformStartTime();");
                    }
                    else
                    {
                        SyntaxError(nextToken);
                    }
                }
                else if (nextToken == "AreaLightSource")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Illuminant, pbrtAreaLightSource);");
                }
                else if (nextToken == "Accelerator")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtAccelerator);");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'C':
                if (nextToken == "ConcatTransform")
                {
                    if (DoGetNextToken(TokenRequired) != "[")
                    {
                        SyntaxError(nextToken);
                    }
                    float m[16];
                    for (int i = 0; i < 16; ++i)
                    {
                        m[i] = ParseNumber(DoGetNextToken(TokenRequired));
                    }
                    if (DoGetNextToken(TokenRequired) != "]")
                    {
                        SyntaxError(nextToken);
                    }
                    printf("pbrtConcatTransform(m);");
                }
                else if (nextToken == "CoordinateSystem")
                {
                    string_view n = dequoteString(DoGetNextToken(TokenRequired));
                    printf("pbrtCoordinateSystem(toString(n));");
                }
                else if (nextToken == "CoordSysTransform")
                {
                    string_view n = dequoteString(DoGetNextToken(TokenRequired));
                    printf("pbrtCoordSysTransform(toString(n));");
                }
                else if (nextToken == "Camera")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, ""pbrtCamera);");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'F':
                if (nextToken == "Film")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtFilm);");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'I':
                if (nextToken == "Integrator")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtIntegrator);");
                }
                else if (nextToken == "Identity")
                {
                    printf("pbrtIdentity();");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'L':
                if (nextToken == "LightSource")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Illuminant, pbrtLightSource);");
                }
                else if (nextToken == "LookAt")
                {
                    float v[9];
                    for (int i = 0; i < 9; ++i)
                    {
                        v[i] = ParseNumber(DoGetNextToken(TokenRequired));
                    }
                    printf("pbrtLookAt(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'M':
                if (nextToken == "MakeNamedMaterial")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtMakeNamedMaterial);");
                }
                else if (nextToken == "MakeNamedMedium")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtMakeNamedMedium);");
                }
                else if (nextToken == "Material")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtMaterial);");
                }
                else if (nextToken == "MediumInterface")
                {
                    string_view n = dequoteString(DoGetNextToken(TokenRequired));
                    std::string names[2];
                    names[0] = n.toString();

                    // Check for optional second parameter
                    string_view second = DoGetNextToken(TokenOptional);
                    if (!second.empty())
                    {
                        if (IsQuotedString(second))
                        {
                            names[1] = dequoteString(second).toString();
                        }
                        else
                        {
                            DoUngetToken(second);
                            names[1] = names[0];
                        }
                    }
                    else
                    {
                        names[1] = names[0];
                    }

                    printf("pbrtMediumInterface(names[0], names[1]);");
                } else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'N':
                if (nextToken == "NamedMaterial")
                {
                    string_view n = dequoteString(DoGetNextToken(TokenRequired));
                    printf("pbrtNamedMaterial(toString(n));");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'O':
                if (nextToken == "ObjectBegin")
                {
                    string_view n = dequoteString(DoGetNextToken(TokenRequired));
                    printf("pbrtObjectBegin(toString(n));");
                }
                else if (nextToken == "ObjectEnd")
                {
                    printf("pbrtObjectEnd();");
                }
                else if (nextToken == "ObjectInstance")
                {
                    string_view n = dequoteString(DoGetNextToken(TokenRequired));
                    printf("pbrtObjectInstance(toString(n));");
                } else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'P':
                if (nextToken == "PixelFilter")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtPixelFilter);");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'R':
                if (nextToken == "ReverseOrientation")
                {
                    printf("pbrtReverseOrientation();");
                }
                else if (nextToken == "Rotate")
                {
                    float v[4];
                    for (int i = 0; i < 4; ++i)
                    {
                        v[i] = ParseNumber(DoGetNextToken(TokenRequired));
                    }
                    printf("pbrtRotate(v[0], v[1], v[2], v[3]);");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'S':
                if (nextToken == "Shape")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtShape);");
                }
                else if (nextToken == "Sampler")
                {
                    printf("basicParamListEntrypoint(SpectrumType::Reflectance, pbrtSampler);");
                }
                else if (nextToken == "Scale")
                {
                    float v[3];
                    for (int i = 0; i < 3; ++i)
                    {
                        v[i] = ParseNumber(DoGetNextToken(TokenRequired));
                    }
                    printf("pbrtScale(v[0], v[1], v[2]);");
                } else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'T':
                if (nextToken == "TransformBegin")
                {
                    printf("pbrtTransformBegin();");
                }
                else if (nextToken == "TransformEnd")
                {
                    printf("pbrtTransformEnd();");
                }
                else if (nextToken == "Transform")
                {
                    if (DoGetNextToken(TokenRequired) != "[")
                    {
                        SyntaxError(nextToken);
                    }
                    float m[16];
                    for (int i = 0; i < 16; ++i)
                    {
                        m[i] = ParseNumber(DoGetNextToken(TokenRequired));
                    }
                    if (DoGetNextToken(TokenRequired) != "]")
                    {
                        SyntaxError(nextToken);
                    }
                    printf("pbrtTransform(m);");
                }
                else if (nextToken == "Translate")
                {
                    float v[3];
                    for (int i = 0; i < 3; ++i)
                    {
                        v[i] = ParseNumber(DoGetNextToken(TokenRequired));
                    }
                    printf("pbrtTranslate(v[0], v[1], v[2]);");
                }
                else if (nextToken == "TransformTimes")
                {
                    float v[2];
                    for (int i = 0; i < 2; ++i)
                    {
                        v[i] = ParseNumber(DoGetNextToken(TokenRequired));
                    }
                    printf("pbrtTransformTimes(v[0], v[1]);");
                }
                else if (nextToken == "Texture")
                {
                    string_view n = dequoteString(DoGetNextToken(TokenRequired));
                    std::string name = n.toString();
                    n = dequoteString(DoGetNextToken(TokenRequired));
                    std::string type = n.toString();

                    printf("Texture");
                    /*basicParamListEntrypoint(SpectrumType::Reflectance,
                        [&](const std::string &texName, const ParamSet &params) {
                            pbrtTexture(name, type, texName, params);
                        });*/
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            case 'W':
                if (nextToken == "WorldBegin")
                {
                    printf("pbrtWorldBegin();");
                }
                else if (nextToken == "WorldEnd")
                {
                    printf("pbrtWorldEnd();");
                }
                else
                {
                    SyntaxError(nextToken);
                }
                break;

            default:
                SyntaxError(nextToken);
            }
        }
    }

    void ParsePBRTFile(std::string filename)
    {
        auto tokError = [](const char *msg) {
            printf("%s", msg);
            exit(1);
        };

        std::unique_ptr<Tokenizer> upTokenizer = Tokenizer::CreateFromFile(filename, tokError);
        if (!upTokenizer)
        {
            return;
        }
        PBRTParse(std::move(upTokenizer));
    }

    void ParsePBRTString(std::string str)
    {
        auto tokError = [](const char *msg) {
            printf("%s", msg);
            exit(1);
        };

        std::unique_ptr<Tokenizer> upTokenizer = Tokenizer::CreateFromString(std::move(str), tokError);
        if (!upTokenizer)
        {
            return;
        }
        PBRTParse(std::move(upTokenizer));
    }

}  // namespace pbrt
