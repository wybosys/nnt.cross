#include "cross.h"
#include "fs.h"
#include "str.h"
#include <regex>

CROSS_BEGIN

#ifdef NNT_WINDOWS

const string PATH_DELIMITER = "\\";

string replace(string const& str, string const& match, string const& tgt)
{
    return regex_replace(str, regex(match), tgt);
}

string normalize(string const& str)
{
    return replace(str, "/", PATH_DELIMITER);
}

bool mkdir(string const& str) 
{
    return ::CreateDirectoryA(str.c_str(), NULL) == S_OK;
}

bool mkdirs(string const& str)
{
    auto strs = explode(normalize(str), PATH_DELIMITER, true);
    if (!strs.size())
        return false;
    auto cur = strs[0];
    size_t idx = 0;
    do {
        if (!exists(cur) && !mkdir(cur))
            return false;
        cur += PATH_DELIMITER + strs[idx++];
    } while (idx <= strs.size());
    return true;
}

bool exists(string const& str) {
    return GetFileAttributesA(str.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool isfile(string const& str) {
    auto f = GetFileAttributesA(str.c_str());
    return INVALID_FILE_ATTRIBUTES != f && 0 == (f & FILE_ATTRIBUTE_DIRECTORY);
}

bool isdirectory(string const& str)
{
    auto f = GetFileAttributesA(str.c_str());
    return INVALID_FILE_ATTRIBUTES != f && 0 != (f & FILE_ATTRIBUTE_DIRECTORY);
}

vector<string> listdir(string const& str) {
    vector<string> r;
    if (!isdirectory(str))
        return r;
    WIN32_FIND_DATAA data;
    auto h = FindFirstFileA(str.c_str(), &data);
    if (h == INVALID_HANDLE_VALUE)
        return r;
    do {
        r.push_back(data.cFileName);
    } while (FindNextFileA(h, &data));
    return r;
}

bool rmfile(string const& str)
{
    return DeleteFileA(str.c_str()) == S_OK;
}

bool rmdir(string const& str)
{
    return RemoveDirectoryA(str.c_str()) == S_OK;
}

string absolute(string const& str)
{
    char buf[BUFSIZ];
    if (S_OK == GetFullPathNameA(str.c_str(), BUFSIZ, buf, NULL))
        return buf;
    return str;
}

#endif

bool rmtree(string const& str)
{
    if (!isdirectory(str))
        return false;
    for (auto &e : listdir(str)) {
        auto cur = str + PATH_DELIMITER + e;
        if (isfile(cur) && !rmfile(cur)) {
            cerr << ("É¾³ý " + cur + " Ê§°Ü") << endl;
            return false;
        }
        else if (!rmtree(cur)) {
            return false;
        }
    }
    return rmdir(str);
}

#ifdef NNT_UNIXLIKE

const string PATH_DELIMITER = "/";

#endif

CROSS_END
