#ifndef NSOUND_SWIG_BUFFER_PROXY_HPP
#define NSOUND_SWIG_BUFFER_PROXY_HPP

#include <Nsound/Buffer.h>


using namespace Nsound;


struct SwigBufferProxy
{
    Buffer & _buf;

    SwigBufferProxy(Buffer & b) : _buf(b) {}

    ~SwigBufferProxy() {}

    void abs()
    {
        _buf.abs();
    }

    Buffer getAbs() const
    {
        return _buf.getAbs();
    }

    void add(const Buffer & buffer, uint32 offset = 0, uint32 n_samples = 0)
    {
        _buf.add(buffer, offset, n_samples);
    }

    uint32 argmax() const
    {
        return _buf.argmax();
    }

    uint32 argmin() const
    {
        return _buf.argmin();
    }

    Buffer::iterator begin()
    {
        return _buf.begin();
    }

    Buffer::const_iterator begin() const
    {
        return _buf.begin();
    }

    Buffer::circular_iterator cbegin()
    {
        Buffer::circular_iterator itor = _buf.cbegin();

        return itor;
    }

    Buffer::const_circular_iterator cbegin() const
    {
        const Buffer & tmp = _buf;
        return tmp.cbegin();
    }

    Buffer::iterator end()
    {
        return _buf.end();
    }

    Buffer::const_iterator end() const
    {
        return _buf.end();
    }

    uint32 getNBytes() const
    {
        return _buf.getNBytes();
    }

    void exp()
    {
        _buf.exp();
    }

    Buffer getExp() const
    {
        return _buf.getExp();
    }

    void convolve(const Buffer & H)
    {
        _buf.convolve(H);
    }

    Buffer getConvolve(const Buffer & H) const
    {
        return _buf.getConvolve(H);
    }

    void dB()
    {
        _buf.dB();
    }

    Buffer getdB() const
    {
        return _buf.getdB();
    }

    void derivative(uint32 n)
    {
        _buf.derivative(n);
    }

    Buffer getDerivative(uint32 n) const
    {
        return _buf.getDerivative(n);
    }

    void downSample(uint32 n)
    {
        _buf.downSample(n);
    }

    Buffer getDownSample(uint32 n) const
    {
        return _buf.getDownSample(n);
    }

    Uint32Vector findPeaks(uint32 window_size = 0, float64 min_height = 0.0) const
    {
        return _buf.findPeaks(window_size, min_height);
    }

    uint32 getLength() const
    {
        return _buf.getLength();
    }

    void limit(float64 min, float64 max)
    {
        _buf.limit(min, max);
    }

    void limit(const Buffer & min, const Buffer & max)
    {
        _buf.limit(min, max);
    }

    Buffer getLimit(float64 min, float64 max) const
    {
        return _buf.getLimit(min, max);
    }

    Buffer getLimit(const Buffer & min, const Buffer & max) const
    {
        return _buf.getLimit(min, max);
    }

    void log()
    {
        _buf.log();
    }

    Buffer getLog() const
    {
        return _buf.getLog();
    }

    void log10()
    {
        _buf.log10();
    }

    Buffer getLog10() const
    {
        return _buf.getLog10();
    }

    float64 getMax() const
    {
        return _buf.getMax();
    }

    float64 getMaxMagnitude() const
    {
        return _buf.getMaxMagnitude();
    }

    float64 getMean() const
    {
        return _buf.getMean();
    }

    float64 getMin() const
    {
        return _buf.getMin();
    }

    void mul(const Buffer & buffer, uint32 offset = 0, uint32 n_samples = 0)
    {
        _buf.mul(buffer, offset, n_samples);
    }

    void normalize()
    {
        _buf.normalize();
    }

    Buffer getNormalize() const
    {
        return _buf.getNormalize();
    }

    Buffer getSignalEnergy(uint32 window_size) const
    {
        return _buf.getSignalEnergy(window_size);
    }

    float64 getStd() const
    {
        return _buf.getStd();
    }

    float64 getSum() const
    {
        return _buf.getSum();
    }

    void zNorm()
    {
        _buf.zNorm();
    }

    Buffer getZNorm() const
    {
        return _buf.getZNorm();
    }

    BufferSelection operator()(const BooleanVector & bv)
    {
        return _buf(bv);
    }

    boolean operator==(const Buffer & rhs) const
    {
        return _buf == rhs;
    }

    boolean operator!=(const Buffer & rhs) const
    {
        return _buf != rhs;
    }

    float64 & operator[](uint32 index)
    {
        return _buf[index];
    }

    const float64 & operator[](uint32 index) const
    {
        return _buf[index];
    }

    SwigBufferProxy & operator<<(const AudioStream & rhs)
    {
        _buf << rhs;
        return *this;
    }

    #define LM_MAKE_OPERATOR( op )                                          \
        SwigBufferProxy & operator op (const Buffer & rhs)                  \
        {                                                                   \
            _buf op rhs;                                                    \
            return *this;                                                   \
        }                                                                   \
                                                                            \
        SwigBufferProxy & operator op (const SwigBufferProxy & rhs)         \
        {                                                                   \
            _buf op rhs._buf;                                               \
            return *this;                                                   \
        }                                                                   \
                                                                            \
        SwigBufferProxy & operator op (float64 rhs)                         \
        {                                                                   \
            _buf op rhs;                                                    \
            return *this;                                                   \
        }

    LM_MAKE_OPERATOR( << )
    LM_MAKE_OPERATOR( += )
    LM_MAKE_OPERATOR( -= )
    LM_MAKE_OPERATOR( *= )
    LM_MAKE_OPERATOR( /= )
    LM_MAKE_OPERATOR( ^= )

    #undef LM_MAKE_OPERATOR

    #define LM_MAKE_OPERATOR( op )                                           \
        BooleanVector operator op (float64 rhs)                              \
        {                                                                    \
            return _buf op rhs;                                              \
        }

    LM_MAKE_OPERATOR( >  )
    LM_MAKE_OPERATOR( >= )
    LM_MAKE_OPERATOR( <  )
    LM_MAKE_OPERATOR( <= )
    LM_MAKE_OPERATOR( == )
    LM_MAKE_OPERATOR( != )

    #undef LM_MAKE_OPERATOR

    void plot(const std::string & title = "Buffer") const
    {
        _buf.plot(title);
    }

    float64 * getPointer()
    {
        return _buf.getPointer();
    }

    const float64 * getPointer() const
    {
        return _buf.getPointer();
    }

    void preallocate(uint32 n)
    {
        _buf.preallocate(n);
    }

    void readWavefile(const char * filename)
    {
        _buf.readWavefile(filename);
    }

    void resample(float64 factor)
    {
        _buf.resample(factor);
    }

    void resample(const Buffer & factor)
    {
        _buf.resample(factor);
    }

    Buffer getResample(float64 factor, const uint32 N = 10, float64 beta = 5.0) const
    {
        return _buf.getResample(factor, N, beta);
    }

    Buffer getResample(const Buffer & factor, const uint32 N = 10, float64 beta = 5.0) const
    {
        return _buf.getResample(factor, N, beta);
    }

    Buffer getResample( const uint32 L, const uint32 M, const uint32 N = 10, float64 beta = 5.0) const
    {
        return _buf.getResample(L, M, N, beta);
    }

    void reverse()
    {
        _buf.reverse();
    }

    Buffer getReverse() const
    {
        return _buf.getReverse();
    }

    void round()
    {
        _buf.round();
    }

    Buffer getRound() const
    {
        return _buf.getRound();
    }

    BufferSelection select(const uint32 start_index, const uint32 stop_index)
    {
        return _buf.select(start_index, stop_index);
    }

    std::string write() const
    {
        return _buf.write();
    }

    void read(const std::string & string_in)
    {
        _buf.read(string_in);
    }

    void smooth(uint32 n_passes, uint32 n_samples_per_average)
    {
        _buf.smooth(n_passes, n_samples_per_average);
    }

    Buffer getSmooth(uint32 n_passes, uint32 n_samples_per_average) const
    {
        return _buf.getSmooth(n_passes, n_samples_per_average);
    }

    void speedUp(float64 step_size)
    {
        _buf.speedUp(step_size);
    }

    Buffer getSpeedUp(float64 step_size) const
    {
        return _buf.getSpeedUp(step_size);
    }

    void speedUp(const Buffer & step_size)
    {
        _buf.speedUp(step_size);
    }

    Buffer getSpeedUp(const Buffer & step_size) const
    {
        return _buf.getSpeedUp(step_size);
    }

    void sqrt()
    {
        _buf.sqrt();
    }

    Buffer getSqrt() const
    {
        return _buf.getSqrt();
    }

    Buffer subbuffer(uint32 start_index, uint32 n_samples = 0) const
    {
        return _buf.subbuffer(start_index, n_samples);
    }

    void upSample(uint32 n)
    {
        _buf.upSample(n);
    }

    Buffer getUpSample(uint32 n) const
    {
        return _buf.getUpSample(n);
    }

    void writeWavefile(const char * filename) const
    {
        _buf.writeWavefile(filename);
    }

    void _set_at_index(int32 index, float64 x)
    {
        _buf[index] = x;
    }

    #define LM_MAKE_OPERATOR( op )                                           \
        inline Buffer operator op (const SwigBufferProxy & rhs)              \
        {                                                                    \
            return _buf op rhs._buf;                                         \
        }                                                                    \
                                                                             \
        inline Buffer operator op (float64 rhs)                              \
        {                                                                    \
            return _buf op rhs;                                              \
        }

    LM_MAKE_OPERATOR( + )
    LM_MAKE_OPERATOR( - )
    LM_MAKE_OPERATOR( * )
    LM_MAKE_OPERATOR( / )
    LM_MAKE_OPERATOR( ^ )

    #undef LM_MAKE_OPERATOR

    #define LM_MAKE_OPERATOR( op_name, op )                                  \
        inline Buffer op_name (float64 lhs)                                  \
        {                                                                    \
            return lhs op _buf;                                              \
        }

    LM_MAKE_OPERATOR( _radd, + )
    LM_MAKE_OPERATOR( _rsub, - )
    LM_MAKE_OPERATOR( _rmul, * )
    LM_MAKE_OPERATOR( _rdiv, / )
    LM_MAKE_OPERATOR( _rxor, ^ )

    #undef LM_MAKE_OPERATOR

    void _swig_shadow() {};

    Buffer * _to_buffer()
    {
        return &_buf;
    }
};


#endif
