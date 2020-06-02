#include "cross.hpp"
#include "fs.hpp"
#include "str.hpp"
#include <regex>
#include <sstream>

#ifdef NNT_WINDOWS

#include <Windows.h>

#endif

#ifdef NNT_UNIXLIKE

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#endif

CROSS_BEGIN

#ifdef NNT_WINDOWS

const string PATH_DELIMITER = "\\";

string normalize(string const &str) {
    return replace(str, "/", PATH_DELIMITER);
}

bool mkdir(string const &str)
{
    return CreateDirectoryA(str.c_str(), NULL);
}

bool exists(string const &str)
{
    return GetFileAttributesA(str.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool isfile(string const &str)
{
    auto f = GetFileAttributesA(str.c_str());
    return INVALID_FILE_ATTRIBUTES != f && 0 == (f & FILE_ATTRIBUTE_DIRECTORY);
}

bool isdirectory(string const &str)
{
    auto f = GetFileAttributesA(str.c_str());
    return INVALID_FILE_ATTRIBUTES != f && 0 != (f & FILE_ATTRIBUTE_DIRECTORY);
}

::std::vector<string> listdir(string const& str)
{
    ::std::vector<string> r;
    if (!isdirectory(str))
        return r;
    auto tgt = normalize(str) + PATH_DELIMITER + "*.*";
    WIN32_FIND_DATAA data;
    auto h = FindFirstFileA(tgt.c_str(), &data);
    if (h == INVALID_HANDLE_VALUE)
        return r;
    do
    {
        string cur = data.cFileName;
        if (cur == "." || cur == "..")
            continue;
        r.push_back(data.cFileName);
    } while (FindNextFileA(h, &data));
    FindClose(h);
    return r;
}

bool rmfile(string const &str)
{
    return DeleteFileA(str.c_str());
}

bool rmdir(string const &str)
{
    return RemoveDirectoryA(str.c_str());
}

string absolute(string const &str)
{
    auto nstr = normalize(str);
    char buf[BUFSIZ];
    if (GetFullPathNameA(nstr.c_str(), BUFSIZ, buf, NULL))
        return buf;
    return nstr;
}

bool isabsolute(string const& str)
{
    const size_t l = str.length();
    if (l == 0)
        return false;
    char idr = str[0];
    if (idr == '/' || idr == '\\')
        return true;
    idr = str[1];
    return idr == ':';
}

string pwd()
{
    char buf[MAX_PATH] = { 0 };
    GetCurrentDirectoryA(MAX_PATH, buf);
    return buf;
}

bool cd(string const& str)
{
    return SetCurrentDirectoryA(str.c_str()) == S_OK;
}

#endif

#ifdef NNT_UNIXLIKE

const string PATH_DELIMITER = "/";

string normalize(string const &str) {
    return replace(str, "\\\\", PATH_DELIMITER);
}

bool mkdir(string const &str) {
    return ::mkdir(str.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

bool exists(string const &str) {
    struct stat st = {0};
    return stat(str.c_str(), &st) == 0;
}

bool isfile(string const &str) {
    struct stat st = {0};
    if (stat(str.c_str(), &st))
        return false;
    return S_ISREG(st.st_mode);
}

bool isdirectory(string const &str) {
    struct stat st = {0};
    if (stat(str.c_str(), &st))
        return false;
    return S_ISDIR(st.st_mode);
}

strings listdir(string const &str) {
    strings r;
    if (!isdirectory(str))
        return r;
    auto tgt = normalize(str) + PATH_DELIMITER;
    auto h = opendir(tgt.c_str());
    auto d = readdir(h);
    while (d) {
        string cur = d->d_name;
        if (cur != "." && cur != "..") {
            r.emplace_back(cur);
        }
        d = readdir(h);
    }
    closedir(h);
    return r;
}

bool rmfile(string const &str) {
    return ::unlink(str.c_str()) == 0;
}

bool rmdir(string const &str) {
    return ::rmdir(str.c_str()) == 0;
}

string absolute(string const &str) {
    char buf[PATH_MAX];
    if (::realpath(str.c_str(), buf))
        return buf;
    return str;
}

bool isabsolute(string const &str) {
    const size_t l = str.length();
    if (l == 0)
        return false;
    char idr = str[0];
    if (idr == '/' || idr == '\\' || idr == '~')
        return true;
    return false;
}

string pwd() {
    char buf[PATH_MAX] = {0};
    getcwd(buf, PATH_MAX);
    return buf;
}

bool cd(string const &str) {
    return ::chdir(str.c_str()) == 0;
}

#endif

string dirname(string const &str) {
    auto nstr = normalize(str);
    auto ps = explode(nstr, PATH_DELIMITER, true);
    if (ps.empty())
        return nstr;
    ps.pop_back();
    return implode(ps, PATH_DELIMITER);
}

string replace(string const &str, string const &match, string const &tgt) {
    return ::std::regex_replace(str, ::std::regex(match), tgt);
}

bool mkdirs(string const &str) {
    auto strs = explode(normalize(str), PATH_DELIMITER, true);
    if (strs.empty())
        return false;
    auto cur = strs[0];
    if (!exists(cur) && !mkdir(cur))
        return false;
    for (size_t idx = 1; idx < strs.size(); ++idx) {
        cur += PATH_DELIMITER + strs[idx];
        if (!exists(cur) && !mkdir(cur))
            return false;
    }
    return true;
}

bool rmtree(string const &str) {
    if (!isdirectory(str))
        return false;
    for (auto &e : listdir(str)) {
        auto cur = str + PATH_DELIMITER + e;
        if (isfile(cur)) {
            if (!rmfile(cur)) {
                cerr << ("删除 " + cur + " 失败") << endl;
                return false;
            }
        } else if (!rmtree(cur)) {
            cerr << ("删除 " + cur + " 失败") << endl;
            return false;
        }
    }
    return rmdir(str);
}

string file_get_contents(string const &file) {
    string t;
    file_get_contents(file, t);
    return t;
}

bool file_get_contents(string const &file, string &result) {
#ifdef NNT_WINDOWS
    FILE *fp = NULL;
    fopen_s(&fp, file.c_str(), "rb");
#else
    FILE *fp = fopen(file.c_str(), "r");
#endif

    if (fp == nullptr)
        return false;

    ::std::ostringstream oss;
    char buf[BUFSIZ];
    while (1) {
        size_t readed = fread(buf, 1, BUFSIZ, fp);
        if (readed == 0) {
            break;
        }
        oss.write(buf, readed);
    }
    fclose(fp);
    result = oss.str();
    return true;
}

bool file_put_contents(string const &file, string const &result) {
#ifdef NNT_WINDOWS
    FILE *fp = NULL;
    fopen_s(&fp, file.c_str(), "wb");
#else
    FILE *fp = fopen(file.c_str(), "w");
#endif

    if (fp == nullptr)
        return false;

    char const *buf = result.c_str();
    size_t full = result.length();

    while (1) {
        size_t writed = fwrite(buf, 1, full, fp);
        if (writed == full) {
            break;
        }
        buf += writed;
        full -= writed;
    }

    fclose(fp);
    return true;
}

CROSS_END
