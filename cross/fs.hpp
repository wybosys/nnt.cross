#ifndef __NNT_CROSS_FS_H_INCLUDED
#define __NNT_CROSS_FS_H_INCLUDED

#include "datetime.hpp"

CROSS_BEGIN

extern const string PATH_DELIMITER;

extern NNT_API string normalize(string const &);

extern NNT_API bool mkdir(string const &);

extern NNT_API bool mkdirs(string const &);

extern NNT_API bool exists(string const &);

extern NNT_API bool isfile(string const &);

extern NNT_API bool isdirectory(string const &);

extern NNT_API ::std::vector<string> listdir(string const &);

extern NNT_API bool rmfile(string const &);

extern NNT_API bool mv(string const&, string const&);

extern NNT_API bool rmtree(string const &);

extern NNT_API bool rmdir(string const &);

extern NNT_API string absolute(string const &);

extern NNT_API bool isabsolute(string const &);

extern NNT_API string dirname(string const &);

// 获得工作目录
extern NNT_API string pwd();

// 修改工作目录
extern NNT_API bool cd(string const &);

// 类似php的直接控制文件内容
extern NNT_API bool file_get_contents(string const &file, string &result);

extern NNT_API string file_get_contents(string const &file);

extern NNT_API bool file_put_contents(string const &file, string const &result);

struct NNT_API Stat {
    timestamp_t tm_created;
    timestamp_t tm_modified;
    size_t size;
};

// 目标文件系统数据
extern NNT_API shared_ptr<Stat> stat(string const &target);

// 获得用户目录
extern NNT_API string path_home();

// 设置用户目录
extern NNT_API void path_home(string const&);

// 获得文档目录
extern NNT_API string path_documents();

extern NNT_API void path_documents(string const&);

// 获得临时目录
extern NNT_API string path_tmp();

extern NNT_API void path_tmp(string const&);

// 获得应用资源目录
extern NNT_API string path_app();

CROSS_END

#endif
