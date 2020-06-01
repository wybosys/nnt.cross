#include "cross.hpp"
#include "zip.hpp"
#include "logger.hpp"
#include "fs.hpp"
#include <minizip/unzip.h>

CROSS_BEGIN

static bool doUnZip(unzFile zf, string const& dir) {
    if (zf == nullptr) {
        Logger::Info("打开zip文件失败");
        return false;
    }

    unz_global_info ginfo;
    if (unzGetGlobalInfo(zf, &ginfo) != UNZ_OK) {
        Logger::Info("读取zip文件信息失败");
        unzClose(zf);
        return false;
    }

    // 解压文件
    for (size_t ni = 0;;) {
        unz_file_info finfo;
        char fnmbuf[FILENAME_MAX];
        if (unzGetCurrentFileInfo(zf, &finfo, fnmbuf, FILENAME_MAX, nullptr, 0, nullptr, 0) != UNZ_OK) {
            Logger::Info("读取zip内文件信息失败");
            unzClose(zf);
            return false;
        }

        // 当前路径
        string cur_name(fnmbuf, finfo.size_filename);
        if (*cur_name.rbegin() == '/') {
            // 当前为文件夹
            string cur_dir = absolute(dir + '/' + cur_name);
            mkdirs(cur_dir);
            if (!exists(cur_dir)) {
                Logger::Info("释放zip失败，不能创建文件夹: " + cur_dir);
                unzClose(zf);
                return false;
            }
        }
        else {
            // 释放文件
            if (unzOpenCurrentFile(zf) != UNZ_OK) {
                Logger::Info("打开zip内文件失败: " + cur_name);
                unzClose(zf);
                return false;
            }
            
            string cur_file = absolute(dir + '/' + cur_name);

#ifdef NNT_WINDOWS            
            FILE *fp = nullptr;
            fopen_s(&fp, cur_file.c_str(), "wb");
#else
            FILE *fp = fopen(cur_file.c_str(), "wb");
#endif

            if (fp == nullptr) {
                Logger::Info("释放zip内文件失败: 无法打开目标文件 " + cur_file);
                unzCloseCurrentFile(zf);
                unzClose(zf);
                return false;
            }

            // 读取数据
            while (1) {
                char buf[BUFSIZ];
                int readed = unzReadCurrentFile(zf, buf, BUFSIZ);

                if (readed < 0) {
                    Logger::Info("释放zip内文件失败: 读取文件内容 " + cur_file);
                    unzCloseCurrentFile(zf);
                    unzClose(zf);
                    fclose(fp);
                    return false;
                }

                if (readed == 0) {
                    unzCloseCurrentFile(zf);
                    break;
                }

                fwrite(buf, 1, readed, fp);
            };

            fclose(fp);
            unzCloseCurrentFile(zf);
        }

        // 读取下一个
        if (++ni < ginfo.number_entry) {
            if (unzGoToNextFile(zf) != UNZ_OK) {
                Logger::Info("读取zip内下一个文件失败");
                unzClose(zf);
                return false;
            }
        }
        else {
            break;
        }
    }

    unzClose(zf);
    return true;
}

bool unzip(string const& ar, string const& dir) {
    unzFile zf = unzOpen(ar.c_str());
    if (doUnZip(zf, dir))
        return true;
    Logger::Info("解压zip文件失败: " + ar);
    return false;
}

bool unzip(char const* buf, size_t lbuf, string const& dir) {
    unzFile zf = unzOpenBuffer(buf, lbuf);
    if (doUnZip(zf, dir))
        return true;
    Logger::Info("解压zip数据流失败");
    return true;
}

CROSS_END
