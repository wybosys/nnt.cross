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

// 临时缓存
class CasualByteBuffer
{
public:

    CasualByteBuffer()
        : _buf(nullptr), _size(0)
    {}

    CasualByteBuffer(char const* buf, size_t lbuf)
        : _buf((void*)buf), _size(lbuf)
    {}

    inline char const* buf() const {
        return (char const*)_buf;
    }

    inline size_t size() const {
        return _size;
    }

    inline string str() const {
        return string(buf(), size());
    }

protected:
    void*_buf;
    size_t _size;
};

// 持久缓存
class ByteBuffer
    : public CasualByteBuffer
{
public:

    ByteBuffer() {}

    ByteBuffer(char const* buf, size_t lbuf)
    {
        _init(buf, lbuf);
    }

    ByteBuffer(ByteBuffer const& r)
    {
        _init(r.buf(), r.size());
    }

    ByteBuffer(CasualByteBuffer const& r)
    {
        _init(r.buf(), r.size());
    }

    ~ByteBuffer()
    {
        free(_buf);
    }

    ByteBuffer& operator = (ByteBuffer const& r)
    {
        _buf = realloc(_buf, r.size());
        memcpy(_buf, r.buf(), r.size());
        return *this;
    }

private:

    void _init(char const* buf, size_t lbuf)
    {
        if (!lbuf)
            return;
        _buf = malloc(lbuf);
        memcpy(_buf, buf, lbuf);
    }
};

template <size_t Count = 8192>
class ByteStream
{
    NNT_NOCOPY(ByteStream);

public:

    typedef char byte_type;
    enum { COUNT = Count };

    ByteStream()
    {
        _buf = malloc(COUNT);
    }

    ByteStream(void const* buf, size_t lbuf)
    {
        while (lbuf > _capacity) {
            _capacity += COUNT;
        }

        _buf = malloc(_capacity);
        memcpy((char*)_buf, buf, lbuf);
        _length = lbuf;
    }

    ~ByteStream()
    {
        free(_buf);
    }

    void write(void const*buf, size_t lbuf)
    {
        bool _extend = false;
        while (_length + lbuf > _capacity) {
            _extend = true;
            _capacity += COUNT;
        }

        if (_extend) {
            _buf = realloc(_buf, _capacity);
        }

        memcpy((char*)_buf + _length, buf, lbuf);
        _length += lbuf;
    }

    void reserve(size_t tgt) 
    {
        if (tgt <= _capacity)
            return;

        _capacity = tgt;
        _buf = realloc(_buf, _capacity);
    }

    inline string str() const {
        return string((char const*)_buf, _length);
    }

    inline size_t size() const {
        return _length;
    }

    inline void clear() {
        _length = 0;
    }

    inline char const* buf() const {
        return (char const*)_buf;
    }

private:
    void* _buf;
    size_t _capacity = COUNT;
    size_t _length = 0;
};

template <typename TStm>
class ByteStreamReader
{
public:

    ByteStreamReader()
        : _stm(::NNT_NS::Nil<TStm>()), _flow(0), _fsize(0)
    {}

    ByteStreamReader(TStm const& stm, size_t low, size_t size)
        : _stm(stm), _flow(low), _fsize(size)
    {}

    ByteStreamReader(TStm const& stm)
        : _stm(stm), _flow(0), _fsize(stm.size())
    {}

    ByteStreamReader(ByteStreamReader const& r)
        : _stm(r._stm), _flow(r._flow), _fsize(r._fsize)
    {}

    inline string str() const {
        return string(_stm.buf() + _flow + _pos_s(), _length_s());
    }

    inline size_t left() const {
        return _length_s();
    }

    inline size_t size() const {
        return _fsize;
    }

    size_t pos = 0;

    inline bool eof() const {
        return pos >= _fsize;
    }

    CasualByteBuffer read(size_t len) {
        auto buf = _stm.buf();
        if (pos + len > _fsize) {
            pos = _fsize;
            return CasualByteBuffer();
        }

        CasualByteBuffer r(buf + pos, len);
        pos += len;
        return r;
    }
    
protected:

    inline size_t _pos_s() const {
        if (pos < 0)
            return 0;
        if (pos >= _fsize)
            return _fsize;
        return pos;
    }

    inline size_t _length_s() const {
        if (pos < 0)
            return _fsize;
        if (pos >= _fsize)
            return 0;
        return _fsize - pos;
    }

private:
    TStm const& _stm;
    size_t const _flow;
    size_t const _fsize;
};

CROSS_END

#endif