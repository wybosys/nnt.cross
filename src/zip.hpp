#ifndef __NNTCROSS_ZIP_H_INCLUDED
#define __NNTCROSS_ZIP_H_INCLUDED

CROSS_BEGIN

// ��ѹ��Ŀ¼�У�����Ѿ����ڣ���Ĭ��Ϊ���ǣ�����޷����ǣ��򷵻ش���
extern NNT_API bool unzip(string const& ar, string const& dir);

extern NNT_API bool unzip(char const*, size_t, string const& dir);

CROSS_END

#endif