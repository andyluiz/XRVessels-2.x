#pragma once

#ifdef _WIN32
#include <atlstr.h>
#else
#include <cstdio>
#include <string>
#include <utility>
#include <strings.h>
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

// The vessel code stores byte-oriented Orbiter paths and log messages in CString.
// Keep those as UTF-8 byte strings on Linux rather than converting paths to UTF-16.
class CString : public std::string
{
public:
    using std::string::string;
    using std::string::operator=;
    CString() = default;
    CString(const std::string &s) : std::string(s) {}

    operator const char *() const { return c_str(); }
    bool IsEmpty() const { return empty(); }
    int GetLength() const { return static_cast<int>(size()); }
    void Empty() { clear(); }
    int Find(char c) const { const auto pos = find(c); return pos == npos ? -1 : static_cast<int>(pos); }
    int ReverseFind(char c) const { const auto pos = rfind(c); return pos == npos ? -1 : static_cast<int>(pos); }
    void Insert(int pos, char c) { insert(static_cast<size_t>(pos), 1, c); }

    char *GetBuffer(size_t length) { resize(length); return data(); }
    char *GetBufferSetLength(size_t length) { return GetBuffer(length); }
    void ReleaseBuffer() { resize(std::char_traits<char>::length(c_str())); }

    template<typename... Args>
    void Format(const char *format, Args... args)
    {
        const int length = std::snprintf(nullptr, 0, format, formatArg(args)...);
        if (length < 0) { clear(); return; }
        resize(static_cast<size_t>(length) + 1);
        std::snprintf(data(), size(), format, formatArg(args)...);
        resize(static_cast<size_t>(length));
    }

private:
    static const char *formatArg(const CString &s) { return s.c_str(); }
    template<typename T> static T formatArg(T value) { return value; }
};

using CStringA = CString;
#endif
