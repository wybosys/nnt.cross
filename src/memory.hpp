#ifndef __NNTCROSS_MEMORY_H_INCLUDED
#define __NNTCROSS_MEMORY_H_INCLUDED

#include <cstring>
#include <functional>
#include <algorithm>

CROSS_BEGIN

template<size_t LEN>
class FixedBuffer {
public:

    typedef FixedBuffer<LEN> self_type;
    enum {
        LENGTH = LEN
    };

    void write(char const *buf, size_t len) {
        while (len) {
            size_t space = LENGTH - _pos;
            size_t writed = ::std::min(space, len);
            memcpy(_buf + _pos, buf, writed);
            _pos += writed;
            len -= writed;
            space -= writed;
            buf += writed;
            if (space == 0) {
                if (proc_full)
                    proc_full(*this);
                _pos = 0;
            }
        }
    }

    ::std::function<void(self_type &)> proc_full;

    inline size_t size() const {
        return _pos;
    }

    inline char const *buf() const {
        return _buf;
    }

    inline void clear() {
        _pos = 0;
    }

private:
    char _buf[LEN];
    size_t _pos = 0;
};

CROSS_END

#endif
