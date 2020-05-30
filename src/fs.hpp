#ifndef __NNT_CROSS_FS_H_INCLUDED
#define __NNT_CROSS_FS_H_INCLUDED

CROSS_BEGIN

extern const string PATH_DELIMITER;

extern NNT_API string replace(string const&, string const& match, string const& tgt);
extern NNT_API string normalize(string const&);
extern NNT_API bool mkdir(string const&);
extern NNT_API bool mkdirs(string const&);
extern NNT_API bool exists(string const&);
extern NNT_API bool isfile(string const&);
extern NNT_API bool isdirectory(string const&);
extern NNT_API vector<string> listdir(string const&);
extern NNT_API bool rmfile(string const&);
extern NNT_API bool rmtree(string const&);
extern NNT_API bool rmdir(string const&);
extern NNT_API string absolute(string const&);
extern NNT_API bool isabsolute(string const&);
extern NNT_API string dirname(string const&);
extern NNT_API string pwd();
extern NNT_API bool cd(string const&);

CROSS_END

#endif