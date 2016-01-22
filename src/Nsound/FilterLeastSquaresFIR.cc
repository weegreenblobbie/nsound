//-----------------------------------------------------------------------------
//
//  $Id: FilterLeastSquaresFIR.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Copyright (c) 2010 to Present Nick Hilton
//
//  weegreenblobbie_yahoo_com (replace '_' with '@' and '.')
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//-----------------------------------------------------------------------------

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterLeastSquaresFIR.h>
#include <Nsound/Generator.h>
#include <Nsound/Plotter.h>

#include <cmath>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <sstream>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": ***ERROR: "

//-----------------------------------------------------------------------------
FilterLeastSquaresFIR::
FilterLeastSquaresFIR(
    const float64 & sample_rate,
    uint32 kernel_size,
    const Buffer & freq_axis,
    const Buffer & amplitude_axis,
    const float64 & beta)
    :
    Filter(sample_rate),
    b_(NULL),
    window_(NULL),
    x_history_(NULL),
    x_ptr_(NULL),
    x_end_ptr_(NULL),
    f_axis_(NULL),
    a_axis_(NULL)
{
    kernel_size_ = kernel_size;

    b_ = new float64[kernel_size_];

    x_history_ = new float64[kernel_size_ + 1];
    x_ptr_ = x_history_;
    x_end_ptr_ = x_history_ + kernel_size_ + 1;

    // Create the Kaiser window.
    window_ = new float64[kernel_size_];

    Generator gen(1.0);

    Buffer kaiser = gen.drawWindowKaiser(kernel_size_, beta);

    memcpy(window_, kaiser.getPointer(), sizeof(float64) * kernel_size_);

    // Allocate f & a axis.
    f_axis_ = new Buffer(16);
    a_axis_ = new Buffer(16);

    FilterLeastSquaresFIR::makeKernel(freq_axis, amplitude_axis);
    FilterLeastSquaresFIR::reset();
}

//-----------------------------------------------------------------------------
// This is private and therefor disabled
FilterLeastSquaresFIR::
    FilterLeastSquaresFIR(const FilterLeastSquaresFIR & copy)
    :
    Filter(copy.sample_rate_),
    b_(NULL),
    window_(NULL),
    x_history_(NULL),
    x_ptr_(NULL),
    x_end_ptr_(NULL),
    f_axis_(NULL),
    a_axis_(NULL)
{
    kernel_size_ = copy.kernel_size_;

    b_ = new float64[kernel_size_];

    x_history_ = new float64[kernel_size_ + 1];
    x_ptr_ = x_history_;
    x_end_ptr_ = x_history_ + kernel_size_ + 1;

    // Create the Kaiser window.
    window_ = new float64[kernel_size_];

    // Allocate f & a axis.
    f_axis_ = new Buffer(16);
    a_axis_ = new Buffer(16);

    // do the rest in the oprator=()
    *this = copy;
}

//-----------------------------------------------------------------------------
FilterLeastSquaresFIR::
~FilterLeastSquaresFIR()
{
    delete [] b_;
    delete [] window_;
    delete [] x_history_;
    delete f_axis_;
    delete a_axis_;
}

Buffer
FilterLeastSquaresFIR::
getKernel() const
{
    Buffer k(kernel_size_);
    for(uint32 i = 0; i < kernel_size_; ++i)
    {
        k << b_[i];
    }

    return k;
}

void
FilterLeastSquaresFIR::
setKernel(const Buffer & k)
{
    if(k.getLength() != kernel_size_)
    {
        delete [] b_;
        delete [] window_;
        delete [] x_history_;

        kernel_size_ = k.getLength();

        b_ = new float64[kernel_size_];

        x_history_ = new float64[kernel_size_ + 1];
        x_ptr_ = x_history_;
        x_end_ptr_ = x_history_ + kernel_size_ + 1;

        // Create the Kaiser window.
        window_ = new float64[kernel_size_];

        Generator gen(1.0);

        Buffer kaiser = gen.drawWindowKaiser(kernel_size_, 5.0);

        memcpy(window_, kaiser.getPointer(), sizeof(float64) * kernel_size_);

        FilterLeastSquaresFIR::reset();
    }

    for(uint32 i = 0; i < kernel_size_; ++i)
    {
        b_[i] = k[i];
    }

    return;
}

Buffer
FilterLeastSquaresFIR::
getKernelFrequencies()
{
    return *f_axis_;
}

Buffer
FilterLeastSquaresFIR::
getKernelAmplitudes()
{
    return *a_axis_;
}

AudioStream
FilterLeastSquaresFIR::
filter(const AudioStream & x)
{
    return Filter::filter(x);
}

Buffer
FilterLeastSquaresFIR::
filter(const Buffer & x)
{
    return Filter::filter(x);
}

float64
FilterLeastSquaresFIR::
filter(const float64 & x)
{
    // Write x to history
    *x_ptr_ = x;

    // Increment history pointer
    ++x_ptr_;

    // Bounds check.
    if(x_ptr_ >= x_end_ptr_)
    {
        x_ptr_ = x_history_;
    }

    // Perform the convolution.

    // y[n] = kernel_[0] * x[n]
    //      + kernel_[1] * x[n - 1]
    //      + kernel_[2] * x[n - 2]
    //      ...
    //      + kernel_[N] * x[n - N]

    float64 y = 0.0;
    float64 * history = x_ptr_;
    for(float64 * b = b_; b != b_ + kernel_size_; ++b)
    {
        // When we enter this loop, history is pointing at x[n + 1].
        --history;

        // Bounds check
        if(history < x_history_)
        {
            history = x_end_ptr_ - 1;
        }

        y += *b * *history;
    }

    return y;
}

void
FilterLeastSquaresFIR::
makeKernel(
    const Buffer & freq_axis,
    const Buffer & amplitude_axis)
{
    // Reset the buffers.
    *f_axis_ = Buffer(16);
    *a_axis_ = Buffer(16);

    float64 max_freq = sample_rate_ / 2.0;

    // Validate freq_axis and amplitude_axis.
    if(freq_axis.getLength() != amplitude_axis.getLength() ||
       freq_axis.getLength() % 2 != 0)
    {
        M_THROW("FilterLeastSquaresFIR::makeKernel(): "
            << "freq_axis and amplitude_axis must be same length and even!");

        *f_axis_ << 0.0 << max_freq;
        *a_axis_ << 1.0 << 1.0;

        FilterLeastSquaresFIR::reset();
        return;
    }

    // Veirfy all samples inf freq_axis are in assending order.
    if(freq_axis[0] != 0.0)
    {
        M_THROW("FilterLeastSquaresFIR::makeKernel(): "
             << "freq_axis[0] must be 0.0!");

        *f_axis_ << 0.0 << max_freq;
        *a_axis_ << 1.0 << 1.0;

        FilterLeastSquaresFIR::reset();
        return;
    }

    if(freq_axis[freq_axis.getLength() -1 ] != max_freq)
    {
        M_THROW("FilterLeastSquaresFIR::makeKernel(): "
             << "freq_axis[end] (" << freq_axis[freq_axis.getLength() -1 ]
             << ") must be " << max_freq << "!");

        *f_axis_ << 0.0 << max_freq;
        *a_axis_ << 1.0 << 1.0;

        FilterLeastSquaresFIR::reset();
        return;
    }

    // Test all samples are in assending order.
    Buffer df = freq_axis.getDerivative(1);
    if(df.getMin() < 0.0)
    {
        M_THROW("FilterLeastSquaresFIR::makeKernel(): "
             << "Frequencies in freq_axis must be increasing!");

        *f_axis_ << 0.0 << max_freq;
        *a_axis_ << 1.0 << 1.0;

        FilterLeastSquaresFIR::reset();
        return;
    }

    // All is okay.

    *f_axis_ << (freq_axis / max_freq);
    *a_axis_ << amplitude_axis;

//~    // Debug
//~    std::cout << "faxis:" << *f_axis_ << std::endl;

    Generator gen(1.0);

    Buffer f(*f_axis_/2.0);
    Buffer a(*a_axis_);

    Buffer w = gen.drawLine(freq_axis.getLength() / 2, 1.0, 1.0);

//~    cout << "N = " << kernel_size_ << endl
//~         << "F = " << endl;
//~
//~    for(uint32 i = 0; i < f.getLength(); ++i)
//~    {
//~        cout << "    " << f[i] << endl;
//~    }

//~    cout << "M = " << endl;
//~
//~    for(uint32 i = 0; i < a.getLength(); ++i)
//~    {
//~        cout << "    " << a[i] << endl;
//~    }

//~    cout << "W = " << endl;
//~
//~    for(uint32 i = 0; i < w.getLength(); ++i)
//~    {
//~        cout << "    " << w[i] << endl;
//~    }

    df = f.getDerivative(1);

    // throw out last value
    df = df.subbuffer(0, df.getLength() -1);

//~    cout << "dF = " << endl;

//~    for(uint32 i = 0; i < df.getLength(); ++i)
//~    {
//~        cout << "    " << df[i] << endl;
//~    }

    // Check for full band

    boolean fullband = true;

//~    cerr << CERR_HEADER << endl;

    if(df.getLength() > 1)
    {
        for(uint32 i = 1; i < df.getLength() - 1; i += 2)
        {

//~            cerr << "df[" << i << "] = " << df[i] << endl;

            if(df[i] != 0.0)
            {
                fullband = false;
                break;
            }
        }
    }

//~    cout << "fullband = " << fullband << endl;

    int32 L = (kernel_size_ - 1) / 2;

//~    cout << "L = " << L << endl;

    boolean is_odd = true;

    if(kernel_size_ % 2 == 0)
    {
        is_odd = false;
    }

//~    cout << "Nodd = " << is_odd << endl;

    Buffer m;

    if(L > 0)
    {
        m = gen.drawLine(L, 0, L);
    }

    m << L;

    if(!is_odd)
    {
        m += 0.5;
    }

//~    cout << "length(m) = " << m.getLength() << endl;
//~    cout << "m[0]   = " << m[0] << endl;
//~    cout << "m[end] = " << m[m.getLength()-1] << endl;

    boolean need_matrix = ! fullband;

//~    cout << "need_matrix = " << need_matrix << endl;

    if(need_matrix)
    {
        M_THROW("FilterLeastSquaresFIR::makeKernel(): "
             << "FIXME: need_matix is true!");
    }

    Buffer k(m);

    float64 b0 = 0.0;

    if(is_odd)
    {
        k = m.subbuffer(1, m.getLength()-1);
    }

//~    cout << "length(k) = " << k.getLength() << endl;
//~    cout << "k[0]   = " << k[0] << endl;
//~    cout << "k[end] = " << k[k.getLength()-1] << endl;

    float64 PI_x_2 = 2.0 * M_PI;
//~    float64 PI_x_4 = 4.0 * M_PI;
    float64 PI_x_PI_x_4 = 4.0 * M_PI * M_PI;

    Buffer b = 0.0 * k;
    Buffer kk = k * k;

    for(uint32 i = 0; i < f.getLength(); i += 2)
    {
        float64 t_df = f[i+1] - f[i];

        float64 mm = 0.0;
        float64 b1 = 0.0;

        mm = (a[i+1] - a[i]) / t_df;    // slope
        b1 = a[i] - mm * f[i];          // y-intercept

//~        cout << endl;
//~        cout << "s = " << i+1 << endl;
//~        cout << "m = " << mm << endl;
//~        cout << "b1 = " << b1 << endl;

        if(is_odd)
        {
            b0 += (b1 * t_df + mm / 2.0 * (f[i+1] * f[i+1] - f[i]*f[i]))
                * w[(i+1)/2] * w[(i+1)/2];
        }

//~        cout << "b0 = " << b0 << endl;

        Buffer f1 = PI_x_2 * k * f[i+1];
        Buffer f2 = PI_x_2 * k * f[i];

        std::stringstream ss;

        float64 abs_w2 = std::fabs(w[(i+1)/2] * w[(i+1)/2]);

//~        cout << "W[" << i << "] = " << w[(i+1)/2] << endl;
//~        cout << "abs(W^2) = " << abs_w2 << endl;


        for(uint32 j = 0; j < b.getLength(); ++j)
        {
            b[j] += (mm / PI_x_PI_x_4
                        * (std::cos(f1[j]) - std::cos(f2[j]))
                        / (k[j] * k[j])
                    ) * abs_w2;
        }

        float64 fi  = f[i];
        float64 fi1 = f[i+1];

//~        ss.str("");
//~        ss << "s = " << (i+1) << ", b1";
//~        b.plot(ss.str());

//~        cout << "F[" << (i)   << "] = " << fi  << endl;
//~        cout << "F[" << (i+1) << "] = " << fi1 << endl;

//~        // DEBUG
//~        Buffer SINC;
//~        for(uint32 j = 0; j < b.getLength(); ++j)
//~        {
//~            float64 x = PI_x_2 * k[j] * fi1;
//~
//~            if(x == 0.0) x = M_PI;
//~
//~            SINC << std::sin(x) / x;
//~        }
//~
//~        ss.str("");
//~        ss << "s = " << (i+1) << ", sinc";
//~        SINC.plot(ss.str());

        for(uint32 j = 0; j < b.getLength(); ++j)
        {
            float64 t = PI_x_2 * k[j];
            float64 x = 0.0;
            float64 sinc = 0.0;
            float64 sinc1 = 0.0;

            x = t*fi;

            if(x == 0.0) x = M_PI;

            sinc = std::sin(x) / x;

            x = t*fi1;

            if(x == 0.0) x = M_PI;

            sinc1 = std::sin(x) / x;

            b[j] += (  fi1 * (mm * fi1 + b1) * sinc1
                     - fi  * (mm * fi  + b1) * sinc )
                  * abs_w2;
        }

//~        ss << "s = " << (i+1) << ", f1";
//~        f1.plot(ss.str());
//~        ss.str("");
//~        ss << "s = " << (i+1) << ", f2";
//~        f2.plot(ss.str());
//~
//~        ss.str("");
//~        ss << "s = " << (i+1) << ", b2";
//~        b.plot(ss.str());

    }

//~    cout << "below loop" << endl
//~         << "b0 = " << b0 << endl;

//~    b.plot("below loop: b");

    if(is_odd)
    {
        Buffer t;
        t << b0 << b;
        b = t;
    }

//~    b.plot("below loop: b2");

//~    a = b;
    Buffer h;

    if(! need_matrix)
    {
        a = w[0] * w[0] * 4.0 * b;

        if(is_odd)
        {
            a[0] *= 0.5;
        }
    }

    if(is_odd)
    {
        float64 a0 = a[0];
        Buffer a2 = 0.5 * a.subbuffer(1, a.getLength());
        h = a2.getReverse() << a0 << a2;
    }
    else
    {
        h = a.getReverse() << a;

        h *= 0.5;
    }

//~    h.plot("h");
//~    Plotter::show();

    if(kernel_size_ != h.getLength())
    {
        M_THROW("FilterLeastSquaresFIR::makeKernel(): "
             << "kernel_size_ != h.getLength()!");

        for(uint32 i = 0; i < kernel_size_; ++i)
        {
            b_[i] = 0.0;
        }
    }
    else
    {
        memcpy(b_, h.getPointer(), sizeof(float64)*kernel_size_);
    }

    // Apply window
    for(uint32 i = 0; i < kernel_size_; ++i)
    {
        b_[i] *= window_[i];
    }
}

//-----------------------------------------------------------------------------
FilterLeastSquaresFIR &
FilterLeastSquaresFIR::
operator=(const FilterLeastSquaresFIR & rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    if(kernel_size_ != rhs.kernel_size_)
    {
        delete [] b_;
        delete [] window_;
        delete [] x_history_;

        kernel_size_ = rhs.kernel_size_;

        b_ = new float64[kernel_size_];

        x_history_ = new float64[kernel_size_ + 1];
        x_ptr_ = x_history_;
        x_end_ptr_ = x_history_ + kernel_size_ + 1;

        // Create the Kaiser window.
        window_ = new float64[kernel_size_];
    }

    sample_rate_ = rhs.sample_rate_;

    *f_axis_ = *rhs.f_axis_;
    *a_axis_ = *rhs.a_axis_;

    memcpy(b_,      rhs.b_,      sizeof(float64) * kernel_size_);
    memcpy(window_, rhs.window_, sizeof(float64) * kernel_size_);

    FilterLeastSquaresFIR::reset();

    return *this;
}

void
FilterLeastSquaresFIR::
plot(boolean show_fc, boolean show_phase)
{
    char title[256];
    sprintf(title,
        "Least Square FIR Frequency Response\n"
        "order = %d, sr = %0.1f Hz",
        kernel_size_, sample_rate_);

    Filter::plot(show_phase);

    Plotter pylab;

    uint32 n_rows = 1;

    if(show_phase)
    {
        n_rows = 2;
    }

    if(show_fc)
    {
        pylab.subplot(n_rows, 1, 1);

        pylab.title(title);
    }
}

void
FilterLeastSquaresFIR::
reset()
{
    memset(x_history_, 0, sizeof(float64) * (kernel_size_ + 1));
    x_ptr_ = x_history_;
}

void
FilterLeastSquaresFIR::
setWindow(WindowType type)
{
    Generator gen(1.0);

    Buffer window = gen.drawWindow(kernel_size_, type);

    memcpy(window_, window.getPointer(), sizeof(float64)*kernel_size_);

    // Make new kernel with window.
    Buffer f(*f_axis_);
    Buffer a(*a_axis_);

    f *= 0.5 * sample_rate_;

    makeKernel(f, a);
}
