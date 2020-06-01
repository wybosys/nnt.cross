#ifndef __NNTCROSS_CODE_H_INCLUDED
#define __NNTCROSS_CODE_H_INCLUDED

CROSS_BEGIN

NNT_API enum struct Code
{
    UNKNOWN = -1000,
    EXCEPTION = -999, // ������δ������쳣
    ROUTER_NOT_FOUND = -998, // û���ҵ�·��
    CONTEXT_LOST = -997, // �����Ķ�ʧ
    MODEL_ERROR = -996, // �ָ�ģ��ʧ��
    PARAMETER_NOT_MATCH = -995, // ����������Ҫ��
    NEED_AUTH = -994, // ��Ҫ��½
    TYPE_MISMATCH = -993, // �������ʹ���
    FILESYSTEM_FAILED = -992, // �ļ�ϵͳʧ��
    FILE_NOT_FOUND = -991, // �ļ�������
    ARCHITECT_DISMATCH = -990, // ���벻���ϱ�׼�ܹ�
    SERVER_NOT_FOUND = -989, // û���ҵ�������
    LENGTH_OVERFLOW = -988, // ���ȳ�������
    TARGET_NOT_FOUND = -987, // Ŀ�����û���ҵ�
    PERMISSION_FAILED = -986, // û��Ȩ��
    NOT_IMPLEMENTION = -985, // �ȴ�ʵ��
    ACTION_NOT_FOUND = -984, // û���ҵ�����
    TARGET_EXISTS = -983, // �Ѿ�����
    STATE_FAILED = -982, // ״̬����
    UPLOAD_FAILED = -981, // �ϴ�ʧ��
    MASK_WORD = -980, // �����д�
    SELF_ACTION = -979, // ����Լ����в���
    PASS_FAILED = -978, // ��֤��ƥ��ʧ��
    MEMORY_OVERFLOW = -977, // �������
    AUTH_EXPIRED = -976, // ��Ȩ����
    SIGNATURE_ERROR = -975, // ǩ������
    FORMAT_ERROR = -974, // ��ʽ����
    CONFIG_ERROR = -973, // ���ô���
    PRIVILEGE_ERROR = -972, // Ȩ�޴���
    LIMIT = -971, // �ܵ�����
    PAGED_OVERFLOW = -970, // ������ҳ���ݵĴ�������
    NEED_ITEMS = -969, // ��Ҫ������Ʒ
    DECODE_ERROR = -968, // ����ʧ��
    ENCODE_ERROR = -967, // ����ʧ��

    IM_CHECK_FAILED = -899, // IM�������Ĳ���ʧ��
    IM_NO_RELEATION = -898, // IM���˫�������ڹ�ϵ

    SOCK_WRONG_PORTOCOL = -860, // SOCKET�����˴����ͨѶЭ��
    SOCK_AUTH_TIMEOUT = -859, // ��Ϊ���Ӻ���û�е�¼�����Է���������Ͽ�������
    SOCK_SERVER_CLOSED = -858, // �������ر�

    SECURITY_FAILED = -6, // ��⵽��ȫ����
    THIRD_FAILED = -5, // ����������
    MULTIDEVICE = -4, // ��˵�½
    HFDENY = -3, // ��Ƶ���ñ��ܾ���֮ǰ�ķ��ʻ�û�н���) high frequency deny
    TIMEOUT = -2, // ��ʱ
    FAILED = -1, // һ��ʧ��
    OK = 0, // �ɹ�
};

class NNT_API error : public ::std::exception {
public:

    error(string const& msg, int code = (int)Code::FAILED) 
        :_msg(msg), _code(code)
    {}

    error(int code, string const& msg = "")
        :_msg(msg), _code(code)
    {}

    error(string const& msg, Code code = Code::FAILED)
        :_msg(msg), _code((int)code)
    {}

    error(Code code, string const& msg = "")
        :_msg(msg), _code((int)code)
    {}

    virtual char const* what() const noexcept {
        return _msg.c_str();
    }

    virtual int code() const noexcept {
        return _code;
    }

    inline operator Code () const noexcept {
        return (Code)_code;
    }

    inline operator int() const noexcept {
        return _code;
    }

private:
    string _msg;
    int _code;
};

CROSS_END

#endif