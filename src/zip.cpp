#include "cross.hpp"
#include "zip.hpp"
#include "logger.hpp"
#include "fs.hpp"
#include <minizip/unzip.h>

CROSS_BEGIN

static bool doUnZip(unzFile zf, string const& dir) {
    if (zf == nullptr) {
        Logger::Info("��zip�ļ�ʧ��");
        return false;
    }

    unz_global_info ginfo;
    if (unzGetGlobalInfo(zf, &ginfo) != UNZ_OK) {
        Logger::Info("��ȡzip�ļ���Ϣʧ��");
        unzClose(zf);
        return false;
    }

    // ��ѹ�ļ�
    for (size_t ni = 0;;) {
        unz_file_info finfo;
        char fnmbuf[FILENAME_MAX];
        if (unzGetCurrentFileInfo(zf, &finfo, fnmbuf, FILENAME_MAX, nullptr, 0, nullptr, 0) != UNZ_OK) {
            Logger::Info("��ȡzip���ļ���Ϣʧ��");
            unzClose(zf);
            return false;
        }

        // ��ǰ·��
        string cur_name(fnmbuf, finfo.size_filename);
        if (*cur_name.rbegin() == '/') {
            // ��ǰΪ�ļ���
            string cur_dir = absolute(dir + '/' + cur_name);
            mkdirs(cur_dir);
            if (!exists(cur_dir)) {
                Logger::Info("�ͷ�zipʧ�ܣ����ܴ����ļ���: " + cur_dir);
                unzClose(zf);
                return false;
            }
        }
        else {
            // �ͷ��ļ�
            if (unzOpenCurrentFile(zf) != UNZ_OK) {
                Logger::Info("��zip���ļ�ʧ��: " + cur_name);
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
                Logger::Info("�ͷ�zip���ļ�ʧ��: �޷���Ŀ���ļ� " + cur_file);
                unzCloseCurrentFile(zf);
                unzClose(zf);
                return false;
            }

            // ��ȡ����
            while (1) {
                char buf[BUFSIZ];
                int readed = unzReadCurrentFile(zf, buf, BUFSIZ);

                if (readed < 0) {
                    Logger::Info("�ͷ�zip���ļ�ʧ��: ��ȡ�ļ����� " + cur_file);
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

        // ��ȡ��һ��
        if (++ni < ginfo.number_entry) {
            if (unzGoToNextFile(zf) != UNZ_OK) {
                Logger::Info("��ȡzip����һ���ļ�ʧ��");
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
    Logger::Info("��ѹzip�ļ�ʧ��: " + ar);
    return false;
}

bool unzip(char const* buf, size_t lbuf, string const& dir) {
    unzFile zf = unzOpenBuffer(buf, lbuf);
    if (doUnZip(zf, dir))
        return true;
    Logger::Info("��ѹzip������ʧ��");
    return true;
}

CROSS_END
