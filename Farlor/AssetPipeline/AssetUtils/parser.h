
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
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace pbrt
{
    // Loc represents a position in a file being parsed.
    struct Loc
    {
        Loc() = default;
        Loc(const std::string& filename) : filename(filename) {}

        std::string filename;
        int line = 1;
        int column = 0;
    };

    // If not nullptr, stores the current file location of the parser.
    // I do not like this.
    extern Loc* g_pParserLoc;

    // TODO: Replace this once we are c++17 supported compiler
    class string_view
    {
      public:
        string_view(const char* pStart, size_t size)
            : m_pPos(pStart)
            , m_length(size)
        {}
        
        string_view()
            : m_pPos(nullptr)
            , m_length(0)
        {}

        const char* data() const { return m_pPos; }
        size_t size() const { return m_length; }
        bool empty() const { return m_length == 0; }

        char operator[](int index) const { return m_pPos[index]; }
        char back() const { return m_pPos[m_length - 1]; }

        const char* begin() const { return m_pPos; }
        const char* end() const { return m_pPos + m_length; }

        bool operator==(const char* pComp) const
        {
            int index;
            for (index = 0; *pComp; ++index, ++pComp)
            {
                if (index >= m_length)
                {
                    return false;
                }
                if (*pComp != m_pPos[index])
                {
                    return false;
                }
            }
            return index == m_length;
        }
        bool operator!=(const char *str) const { return !(*this == str); }

        void remove_prefix(int n)
        {
            m_pPos += n;
            m_length -= n;
        }
        
        void remove_suffix(int n) { m_length -= n; }

        std::string toString()
        {
            return std::string(m_pPos, m_length);
        }

      private:
        const char* m_pPos;
        size_t m_length;
    };

    // Tokenizer converts a single pbrt scene file into a series of tokens.
    class Tokenizer
    {
    public:
        static std::unique_ptr<Tokenizer> CreateFromFile(const std::string& filename, std::function<void(const char*)> errorCallback);
        static std::unique_ptr<Tokenizer> CreateFromString(std::string str, std::function<void(const char*)> errorCallback);

        ~Tokenizer();

        // Returns an empty string_view at EOF. Note that the returned
        // string_view is not guaranteed to be valid after next call to Next().
        string_view NextToken();

    public:
        Loc m_currentLoc;

    private:
        // Only use a factory method to create a pbrt tokenizer
        Tokenizer(std::string str, std::function<void(const char *)> errorCallback);

        int GetNextChar()
        {
            // We are at the end of the file
            if (m_pCurrentPos == m_pEndPos)
            {
                return EOF;
            }

            // We always get the current position, then push forward for the next grab
            int ch = *m_pCurrentPos;
            m_pCurrentPos++;

            if (ch == '\n')
            {
                ++m_currentLoc.line;
                m_currentLoc.column = 0;
            }
            else
            {
                ++m_currentLoc.column;
            }
            return ch;
        }

        void PutbackChar()
        {
            --m_pCurrentPos;
            if (*m_pCurrentPos == '\n')
            {
                // Don't worry about the column; we'll be going to the start of
                // the next line again shortly...
                --m_currentLoc.line;
            }

            // TODO: Ensure that m_current loc is set to the right spot...?
        }
        
    private:
        // This function is called if there is an error during lexing.
        std::function<void(const char*)> m_errorCallback;

        // If the input is stdin, then we copy everything until EOF into this
        // string and then start lexing.  This is a little wasteful (versus
        // tokenizing directly from stdin), but makes the implementation
        // simpler.
        std::string m_contents;

        // Pointers to the current position in the file and one past the end of
        // the file.
        const char* m_pCurrentPos;
        const char* m_pEndPos;

        // If there are escaped characters in the string, we can't just return
        // a string_view into the mapped file. In that case, we handle the
        // escaped characters and return a string_view to sEscaped.  (And
        // thence, string_views from previous calls to Next() must be invalid
        // after a subsequent call, since we may reuse sEscaped.)
        std::string m_processedEscaped;
    };
}  // namespace pbrt