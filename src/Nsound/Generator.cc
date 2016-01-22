//-----------------------------------------------------------------------------
//
//  $Id: Generator.cc 912 2015-07-26 00:50:29Z weegreenblobbie $
//
//  Copyright (c) 2005-2006 Nick Hilton
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

#include <Nsound/Buffer.h>
#include <Nsound/Generator.h>
#include <Nsound/RngTausworthe.h>

#include <iostream>

//~#include <cmath>
#include <string.h>

#include <math.h>

using namespace Nsound;

using std::cout;
using std::cerr;
using std::endl;

#define M_2PI (2.0*M_PI)

#define CERR_HEADER __FILE__ << ":" << __LINE__ << ": "

// Constructor
Generator::
Generator()
    :
    last_frequency_(0.0),
    position_(0.0),
    sync_pos_(0.0),
    sample_rate_(0.0),
    sample_time_(0.0),
    t_(0.0),
    waveform_(NULL),
    rng_(NULL),
    buzz_max_harmonics_(0),
    buzz_position_(),
    chorus_is_on_(false),
    chorus_n_voices_(0),
    chorus_position_(),
    chorus_factor_(),
    sync_is_master_(false),
    sync_is_slave_(false),
    sync_count_(0),
    sync_vector_(),
    sync_slaves_()
{
};

// Constructor
Generator::
Generator(const float64 & sample_rate)
    :
    is_realtime_(false),
    last_frequency_(0.0),
    position_(0.0),
    sync_pos_(0.0),
    sample_rate_(0.0),
    sample_time_(0.0),
    t_(0.0),
    waveform_(NULL),
    rng_(NULL),
    buzz_max_harmonics_(0),
    buzz_position_(),
    chorus_is_on_(false),
    chorus_n_voices_(0),
    chorus_position_(),
    chorus_factor_(),
    sync_is_master_(false),
    sync_is_slave_(false),
    sync_count_(0),
    sync_vector_(),
    sync_slaves_()
{
    ctor(sample_rate);
}

// Constructor
Generator::
Generator(const std::string & wave_filename)
    :
    is_realtime_(false),
    last_frequency_(0.0),
    position_(0.0),
    sync_pos_(0.0),
    sample_rate_(0.0),
    sample_time_(0.0),
    t_(0.0),
    waveform_(NULL),
    rng_(NULL),
    buzz_max_harmonics_(0),
    buzz_position_(),
    chorus_is_on_(false),
    chorus_n_voices_(0),
    chorus_position_(),
    chorus_factor_(),
    sync_is_master_(false),
    sync_is_slave_(false),
    sync_count_(0),
    sync_vector_(),
    sync_slaves_()
{
    Buffer b(wave_filename);
    ctor(b.getLength(), b);
}

// Constructor
Generator::
Generator(const float64 & sample_rate, const Buffer & waveform)
    :
    is_realtime_(false),
    last_frequency_(0.0),
    position_(0.0),
    sync_pos_(0.0),
    sample_rate_(0.0),
    sample_time_(0.0),
    t_(0.0),
    waveform_(NULL),
    rng_(NULL),
    buzz_max_harmonics_(0),
    buzz_position_(),
    chorus_is_on_(false),
    chorus_n_voices_(0),
    chorus_position_(),
    chorus_factor_(),
    sync_is_master_(false),
    sync_is_slave_(false),
    sync_count_(0),
    sync_vector_(),
    sync_slaves_()
{
    ctor(sample_rate, waveform);
}

// Copy Constructor
Generator::
Generator(const Generator & copy)
    :
    is_realtime_(false),
    last_frequency_(0.0),
    position_(0.0),
    sync_pos_(0.0),
    sample_rate_(0.0),
    sample_time_(0.0),
    t_(0.0),
    waveform_(NULL),
    rng_(NULL),
    buzz_max_harmonics_(0),
    buzz_position_(),
    chorus_is_on_(false),
    chorus_n_voices_(0),
    chorus_position_(),
    chorus_factor_(),
    sync_is_master_(false),
    sync_is_slave_(false),
    sync_count_(0),
    sync_vector_(),
    sync_slaves_()
{
    // Call operator=
    *this = copy;
}

// Destructor
Generator::
~Generator()
{
    delete waveform_;
    delete rng_;
};

// ctor
void
Generator::
ctor(const float64 & sample_rate)
{
    sample_rate_ = sample_rate;
    sample_time_ = 1.0 / sample_rate_;
    waveform_ = NULL;
    rng_ = new RngTausworthe();
}

// ctor
void
Generator::
ctor(const float64 & sample_rate, const Buffer & waveform)
{
    if(waveform.getLength() != sample_rate)
    {
        M_THROW("Generator::ctor(): waveform.getLength() != sample_rate ("
             << waveform.getLength()
             << " != "
             << sample_rate
             << ")");
        ctor(sample_rate);
    }
    else
    {
        delete waveform_;
        delete rng_;

        sample_rate_ = sample_rate;
        sample_time_ = 1.0 / sample_rate_;
        waveform_ = new Buffer(waveform);
        rng_ = new RngTausworthe();
    }
}

void
Generator::
addSlaveSync(Generator & slave)
{
    sync_is_master_ = true;
    slave.sync_is_slave_ = true;

    if(sync_slaves_.count(&slave) == 0)
    {
        sync_slaves_.insert(&slave);
    }

    reset();
}

void
Generator::
buzzInit(const uint32 & max_harmonics)
{
    buzz_max_harmonics_ = max_harmonics;
    reset();
}

float64
Generator::
buzz(
    const float64 & frequency,
    const float64 & n_harmonics,
    const float64 & phase_offset)
{
    // This function is based on the csound buzz opcode.

    float64 n = static_cast<int32>(n_harmonics);

    n = fabs(n);
    if(n < 1.0) n = 1.0;

    float64 two_n_plus_1 = 2.0 * n + 1.0;
    float64 scale = static_cast<float64>(0.5) / n;

    float64 sign = 1.0;
    for(int i = 0; i < n; ++i) sign = -sign;

    float64 phase = fabs(phase_offset / static_cast<float64>(2.0));

    while(phase >= 0.5) phase -= 0.5;

    position_ += (phase - last_frequency_) * sample_rate_;

    last_frequency_ = phase;

    float64 y;

    while(position_ >= sample_rate_) position_ -= sample_rate_;

    float64 denom = (*waveform_)[static_cast<int32>(position_ + 0.5)] * sign;

    if(fabs(denom) > 1e-12)
    {
        float64 up_phase = position_ * two_n_plus_1;
        while(up_phase >= sample_rate_) up_phase -= sample_rate_;

        float64 num = (*waveform_)[static_cast<int32>(up_phase)];

        y = (num / denom - static_cast<float64>(1.0)) * scale;
    }
    else
    {
        y = 1.0; // this assumes cosine wave!
    }

    position_ += 0.5*frequency;

    return y;
}

Buffer
Generator::
buzz(
    const float64 & duration,
    const float64 & frequency,
    const float64 & n_harmonics,
    const float64 & phase_offset)
{
    buzzInit(static_cast<uint32>(n_harmonics));

    int32 n_samples = static_cast<int32>(duration * sample_rate_);

    Buffer y(n_samples);

    for(int i = 0; i < n_samples; ++i)
    {
        y << buzz(frequency, n_harmonics, phase_offset+0.5);
    }

    return y;
}

Buffer
Generator::
buzz(
    const float64 & duration,
    const Buffer &  frequencies,
    const Buffer &  n_harmonics,
    const Buffer &  phase_offset)
{
    buzzInit(static_cast<uint32>(n_harmonics.getMax()));

    int32 n_samples = static_cast<int32>(duration * sample_rate_);

    Buffer::const_circular_iterator f = frequencies.cbegin();
    Buffer::const_circular_iterator n = n_harmonics.cbegin();
    Buffer::const_circular_iterator p = phase_offset.cbegin();

    Buffer y(n_samples);

    for(int32 i = 0; i < n_samples; ++i, ++f, ++n, ++p)
    {
        y << buzz(*f, *n, *p+0.5);
    }

    return y;
}

void
Generator::
setChorus(const uint32 n_voices, const float64 & amount)
{
    if(n_voices == 0)
    {
        chorus_is_on_ = false;
        return;
    }

    chorus_is_on_ = true;

    chorus_n_voices_ = n_voices;

    chorus_factor_.clear();
    chorus_position_.clear();

    for(uint32 i = 0; i < chorus_n_voices_; ++i)
    {
        chorus_factor_.push_back(1.0 + rng_->get(-amount, amount));

        chorus_position_.push_back(0.0);
    }

    reset();
}

Buffer
Generator::
drawDecay(const float64 & duration, const float64 & alpha) const
{
    if(duration <= 0.0) return Buffer();

    Buffer t = drawLine(duration, 0.0, 1.0);

    t *= -alpha;

    t.exp();

    return t;
}

Buffer
Generator::
drawGaussian(
    const float64 & duration,
    const float64 & mu,
    const float64 & sigma,
    const boolean & normalize) const
{
    if(duration <= 0.0) return Buffer();

    float64 variance = sigma * sigma;

    Buffer g = drawLine(duration, 0.0, duration);

    g -= mu;

    g *= g;

    g /= (2.0 * variance);

    g *= -1.0;

    g.exp();

    g /= ::sqrt(M_2PI * variance);

    if(normalize) g.normalize();

    return g;
}

Buffer
Generator::
drawFatGaussian(
    const float64 & duration,
    const float64 & pass_band_percent) const
{
    M_ASSERT_VALUE(duration, >, 0.0);
    M_ASSERT_VALUE(pass_band_percent, >, 0.0);
    M_ASSERT_VALUE(pass_band_percent, <, 1.0);

    float64 pass_band_time = duration * pass_band_percent;

    float64 gauss_time = duration - pass_band_time;

    float64 h_gauss_time = gauss_time / 2.0;

    // Using a constant sigma ratio so the shape of the fat gaussian doesn't
    // change given a duration.

    float64 sigma = 0.275 * h_gauss_time;

    Buffer y;

    y << drawGaussian(h_gauss_time, h_gauss_time, sigma)
      << drawLine(pass_band_time, 1.0, 1.0)
      << drawGaussian(h_gauss_time, 0.0, sigma);

    return y;
}

Buffer
Generator::
drawLine(
    const float64 & duration,
    const float64 & y1,
    const float64 & y2) const
{
    M_ASSERT_VALUE(duration, >, 0.0);

    Buffer buffer;

    float64 n_samples = duration * sample_rate_;

    float64 slope = (y2 - y1) / n_samples;

    float64 current_sample = y1;

    for(uint32 i = 0; i < static_cast<uint32>(n_samples+0.5); ++i)
    {
        buffer << current_sample;
        current_sample += slope;
    }

    return buffer;
}

Buffer
Generator::
drawParabola(
    const float64 & duration,
    const float64 & y1,
    const float64 & x2,
    const float64 & y2,
    const float64 & y3) const
{
    M_ASSERT_VALUE(duration, >, 0.0);

    // In this discustion, amp = amplitude.
    //
    // The general equation for a parabola is:
    //
    // y = A*x^2 + B*x + C
    //
    // This alogorithm uses Gaussian elimination to solve for A,B,C.
    //
    // y1 = A * x1^2 + B * x1 + C            (1)
    //
    // y2 = A * x2^2 + B * x2 + C            (2)
    //
    // y3 = A * x3^2 + B * x3 + C            (3)
    //
    // Solve for C using (1), with x1 = 0.0:
    //
    // C = y1
    //
    // Solve for A using (2) and substituting C:
    //
    // A = (y2 - y1 - B * x2) / (x2^2)
    //
    // Solve for B using (3) and substituting A,C, I used WolframAlpha, awesome!
    //
    // solve y_3 = (y_2 - y_1 - B*x_2) / (x_2 * x_2) * (x_3 * x_3) + B*(x_3) + y_1 for B
    //
    //  B = [x2 * x2 * ( y3 - y1 ) + x3 * x3 * (y1 - y2)] / [x2 * x3 * (x2 - x3)]
    //
    M_ASSERT_VALUE(x2, <, duration);

    float64 x3 = duration;

    float64 C = y1;

    float64 B = (  x2*x2*(y3 - y1) + x3*x3*(y1 - y2)  ) / ( x2*x3*(x2 - x3) );

    float64 A = (y2 - y1 - B*x2) / (x2*x2);

    Buffer t = drawLine(duration, 0.0, duration);

    return A*t*t + B*t + C;
}

Buffer
Generator::
drawSine(
    const float64 & duration,
    const float64 & frequency)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    t_ = 0.0;

    Buffer y;

    uint64 n_samples = static_cast<uint64>(duration * sample_rate_ + 0.5);

    for(uint64 i = 0; i < n_samples; ++i)
    {
        y << drawSine2(frequency, 0.0);
    }

    return y;
}

Buffer
Generator::
drawSine(
    const float64 & duration,
    const Buffer & frequency)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    t_ = 0.0;

    Buffer y;

    uint64 n_samples = static_cast<uint64>(duration * sample_rate_ + 0.5);

    Buffer::const_circular_iterator f = frequency.cbegin();

    for(uint64 i = 0; i < n_samples; ++i)
    {
        y << drawSine2(*f, 0.0);
        ++f;
    }

    return y;
}

Buffer
Generator::
drawSine2(
    const float64 & duration,
    const float64 & frequency,
    const float64 & phase)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    t_ = 0.0;

    Buffer y;

    uint64 n_samples = static_cast<uint64>(duration * sample_rate_ + 0.5);

    for(uint64 i = 0; i < n_samples; ++i)
    {
        y << drawSine2(frequency, phase);
    }

    return y;
}

Buffer
Generator::
drawSine2(
    const float64 & duration,
    const Buffer &  frequency,
    const Buffer &  phase)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    t_ = 0.0;

    Buffer y;

    uint64 n_samples = static_cast<uint64>(duration * sample_rate_ + 0.5);

    Buffer::const_circular_iterator f = frequency.cbegin();
    Buffer::const_circular_iterator p = phase.cbegin();

    for(uint64 i = 0; i < n_samples; ++i)
    {
        y << drawSine2(*f, *p);
        ++f;
        ++p;
    }

    return y;
}

Buffer
Generator::
drawSine2(
    const float64 & duration,
    const Buffer &  frequency,
    const float64 & phase)
{
    Buffer p(1);
    p << phase;

    return drawSine2(duration, frequency, p);
}

Buffer
Generator::
drawSine2(
    const float64 & duration,
    const float64 & frequency,
    const Buffer &  phase)
{
    Buffer f(1);
    f << frequency;

    return drawSine2(duration, f, phase);
}

float64
Generator::
drawSine2(const float64 & frequency, const float64 & phase)
{
    float64 f = 0.0;
    float64 sample = 0.0;

    if(chorus_is_on_)
    {
        for(uint32 i = 0; i < chorus_n_voices_; ++i)
        {
            f = M_2PI * chorus_position_[i] * sample_time_ + M_PI * phase;
            sample += ::sin(f);
            chorus_position_[i] += frequency * chorus_factor_[i];
        }
        sample /= static_cast<float64>(chorus_n_voices_);
    }
    else
    {
        sample = ::sin(M_2PI * t_ * sample_time_ + M_PI * phase);
        t_ += frequency;
    }

    return sample;
}

float64
Generator::
drawSine(const float64 & frequency)
{
    return drawSine2(frequency, 0.0);
}


Buffer
Generator::
drawWindow(const float64 & duration, WindowType type) const
{
    switch(type)
    {
        case BARTLETT:        return drawWindowBartlett(duration);
        case BLACKMAN:        return drawWindowBlackman(duration);
        case BLACKMAN_HARRIS: return drawWindowBlackmanHarris(duration);
        case GAUSSIAN:        return drawGaussian(duration, duration/2.0, 1.0);
        case GAUSSIAN_05:     return drawFatGaussian(duration, 0.05);
        case GAUSSIAN_10:     return drawFatGaussian(duration, 0.10);
        case GAUSSIAN_15:     return drawFatGaussian(duration, 0.15);
        case GAUSSIAN_20:     return drawFatGaussian(duration, 0.20);
        case GAUSSIAN_25:     return drawFatGaussian(duration, 0.25);
        case GAUSSIAN_30:     return drawFatGaussian(duration, 0.30);
        case GAUSSIAN_33:     return drawFatGaussian(duration, 0.3333);
        case GAUSSIAN_35:     return drawFatGaussian(duration, 0.35);
        case GAUSSIAN_40:     return drawFatGaussian(duration, 0.40);
        case GAUSSIAN_45:     return drawFatGaussian(duration, 0.45);
        case GAUSSIAN_50:     return drawFatGaussian(duration, 0.50);
        case GAUSSIAN_55:     return drawFatGaussian(duration, 0.55);
        case GAUSSIAN_60:     return drawFatGaussian(duration, 0.60);
        case GAUSSIAN_65:     return drawFatGaussian(duration, 0.65);
        case GAUSSIAN_66:     return drawFatGaussian(duration, 0.6666);
        case GAUSSIAN_70:     return drawFatGaussian(duration, 0.70);
        case GAUSSIAN_75:     return drawFatGaussian(duration, 0.75);
        case GAUSSIAN_80:     return drawFatGaussian(duration, 0.80);
        case GAUSSIAN_85:     return drawFatGaussian(duration, 0.85);
        case GAUSSIAN_90:     return drawFatGaussian(duration, 0.90);
        case GAUSSIAN_95:     return drawFatGaussian(duration, 0.95);
        case GAUSSIAN_99:     return drawFatGaussian(duration, 0.9999);
        case HAMMING:         return drawWindowHamming(duration);
        case HANNING:         return drawWindowHanning(duration);
        case KAISER:          return drawWindowKaiser(duration);
        case NUTTALL:         return drawWindowNuttall(duration);
        case PARZEN:          return drawWindowParzen(duration);
        case RECTANGULAR:     return drawWindowRectangular(duration);
        default:              M_THROW("Unkown window type " << type);
    }

    return drawWindowRectangular(duration);
}

static
void
cosinewindow(
    Buffer & win,
    const float64 & a0,
    const float64 & a1,
    const float64 & a2,
    const float64 & a3)
{
    int32 n = win.getLength();
    for(int32 i = 0; i < n; ++i)
    {
        win[i] *= a0
                - a1 * ::cos(2.0 * M_PI * i / n)
                + a2 * ::cos(4.0 * M_PI * i / n)
                - a3 * ::cos(6.0 * M_PI * i / n);
    }
}

Buffer
Generator::
drawWindowBartlett(const float64 & duration) const
{
    Buffer window = drawLine(duration, 1.0, 1.0);

    int32 n = static_cast<int32>(duration * sample_rate_);

    float64 n_over_2 = static_cast<float64>(n / 2);

    for(int32 i = 0; i < n / 2; ++i)
    {
        float64 t = static_cast<float64>(i) / n_over_2;
        window[i] *= t;
        window[i + n/2] *= 1.0 - t;
    }

    return window;
}

Buffer
Generator::
drawWindowBlackman(const float64 & duration) const
{
    Buffer window = drawLine(duration, 1.0, 1.0);

    cosinewindow(window, 0.42, 0.50, 0.08, 0.00);

    return window;
}

Buffer
Generator::
drawWindowBlackmanHarris(const float64 & duration) const
{
    Buffer window = drawLine(duration, 1.0, 1.0);

    cosinewindow(window, 0.35875, 0.48829, 0.14128, 0.01168);

    return window;
}


Buffer
Generator::
drawWindowHamming(const float64 & duration) const
{
    Buffer window = drawLine(duration, 1.0, 1.0);

    cosinewindow(window, 0.54, 0.46, 0.00, 0.00);

    return window;
}

Buffer
Generator::
drawWindowHanning(const float64 & duration) const
{
    Buffer window = drawLine(duration, 1.0, 1.0);

    cosinewindow(window, 0.50, 0.50, 0.00, 0.00);

    return window;
}

// Computation of the modified Bessel function I_0(x) for any real x.
//
// The algorithm is based on the article by Abramowitz and Stegun [1]
// as denoted in Numerical Recipes 2nd ed. on p. 230 (W.H.Press et al.).
//
// [1] M.Abramowitz and I.A.Stegun, Handbook of Mathematical Functions,
//     Applied Mathematics Series vol. 55 (1964), Washington.
//
float64
bessel_i0(const float64 & x)
{
    const float64 p1 = 1.0,
                  p2 = 3.5156229,
                  p3 = 3.0899424,
                  p4 = 1.2067492,
                  p5 = 0.2659732,
                  p6 = 3.60768e-2,
                  p7 = 4.5813e-3;

    const float64 q1 =  0.39894228,
                  q2 =  1.328592e-2,
                  q3 =  2.25319e-3,
                  q4 = -1.57565e-3,
                  q5 =  9.16281e-3,
                  q6 = -2.057706e-2,
                  q7 =  2.635537e-2,
                  q8 = -1.647633e-2,
                  q9 =  3.92377e-3;

    float64 ax = ::fabs(x);

    float64      y = 0.0,
            result = 0.0;

    if (ax < 3.75)
    {
        y = ::pow(x / 3.75, 2);
        result = p1+y*(p2+y*(p3+y*(p4+y*(p5+y*(p6+y*p7)))));
    }
    else
    {
        y = 3.75 / ax;
        result = (::exp(ax) / ::sqrt(ax))
               * (q1+y*(q2+y*(q3+y*(q4+y*(q5+y*(q6+y*(q7+y*(q8+y*q9))))))));
 }
    return result;
}


Buffer
Generator::
drawWindowKaiser(const float64 & duration, const float64 & beta) const
{
    Buffer window = drawLine(duration, 0.0, 0.0);

    int32 n_samples = window.getLength();
    float64 f_n_samples = float64(n_samples);

    float64 b = ::fabs(beta);

    if(b < 1.0)
    {
        b = 1.0;
    }

    // Calculate window coefficients
    //
    // alpha = n_samples / 2
    //
    // for n = 0 : n_samples -1
    //
    //     w[n] = bessel_i0( beta * sqrt(1.0 - [(n - alpha)/alpha]^2)
    //          / bessel_i0(beta)
    //
    //

    float64 alpha = f_n_samples / 2.0;

    float64 denominator = bessel_i0(b);

    for(int32 i = 0 ; i < n_samples; ++i)
    {
        float64 temp = (float64(i) - alpha) / alpha;
        temp *= temp;

        window[i] = bessel_i0( b * ::sqrt(1.0 - temp)) / denominator;
    }

    return window;
}

Buffer
Generator::
drawWindowNuttall(const float64 & duration) const
{
    Buffer window = drawLine(duration, 1.0, 1.0);

    cosinewindow(window, 0.3635819, 0.4891775, 0.1365995, 0.0106411);

    return window;
}

Buffer
Generator::
drawWindowParzen(const float64 & duration) const
{
    Buffer window = drawLine(duration, 1.0, 1.0);

    int32 n = static_cast<int32>(duration * sample_rate_);

    int32 N = n -1;

    float64 Nf = static_cast<float64>(N);

    for(int32 i = 0; i < N / 4; ++i)
    {
        float64 m = 2 * pow(1.0 - (Nf / 2 - i) / (Nf / 2), 3.0);
        window[i] *= m;
        window[N - i] *= m;
    }

    for(int32 i = N/4; i <= N/2; ++i)
    {
        int32 wn = i - N/2;
        float64 m = 1.0
                  - 6.0 * pow(wn / (Nf/2),2.0)
                      * (1.0 - fabs(static_cast<float64>(wn)) / (Nf/2));

        window[i] *= m;
        window[N-i] *= m;
    }

    return window;
}

Buffer
Generator::
drawWindowRectangular(const float64 & duration) const
{
    return drawLine(duration, 1.0, 1.0);
}

float64
Generator::
generate(const float64 & frequency)
{
    return generate2(frequency, 0.0);
}

float64
Generator::
generate2(const float64 & frequency, const float64 & phase)
{
    M_CHECK_PTR(waveform_);

    ++sync_count_;

    if(sync_is_slave_ && !sync_vector_.empty())
    {
        uint32 sync_count = sync_vector_.front();

        if(sync_count_ == sync_count)
        {
            sync_vector_.erase(sync_vector_.begin());
            position_ = 0;
            sync_pos_ = 0;
        }
    }

    // Move with phase
    float64 ph = (phase * sample_rate_ / 2.0);
    float64 position2 = position_ + ph + 0.5;

    // Range checks

    while(position2 >= sample_rate_)
    {
        position2 -= sample_rate_;
    }

    while(position2 < 0.0)
    {
        position2 += sample_rate_;
    }

    float64 sample = 0.0;

    if(chorus_is_on_)
    {
        for(uint32 i = 0; i < chorus_n_voices_; ++i)
        {
            float64 pos = chorus_position_[i]
                        + chorus_factor_[i] * frequency
                        + ph
                        + 0.5;

            // Range check
            while(pos >= sample_rate_) pos -= sample_rate_;
            while(pos <  0)            pos += sample_rate_;

            sample += (*waveform_)[static_cast<uint32>(pos)];

            chorus_position_[i] += frequency * chorus_factor_[i];
        }

        sample /= static_cast<float64>(chorus_n_voices_);
    }
    else
    {
        sample = (*waveform_)[static_cast<uint32>(position2)];
    }

    position_ += frequency;
    sync_pos_ += frequency;

    // limit
    if(sync_pos_ > sample_rate_)
    {
        sync_pos_ -= sample_rate_;

        if(sync_is_master_)
        {
            std::set<Generator *>::iterator itor = sync_slaves_.begin();

            while(itor != sync_slaves_.end())
            {
                (*itor)->sync_vector_.push_back(sync_count_);
                ++itor;
            }
        }
    }

    return sample;
}

Buffer
Generator::
generate(
    const float64 & duration,
    const float64 & frequency)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    if(!is_realtime_) reset();

    Buffer buffer;

    uint64 n_samples = static_cast<uint64>(std::ceil(duration * sample_rate_));

    for(uint64 i = 0; i < n_samples; ++i)
    {
        buffer << generate(frequency);
    }

    return buffer;
}

Buffer
Generator::
generate2(
    const float64 & duration,
    const float64 & frequency,
    const float64 & phase)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    if(!is_realtime_) reset();

    Buffer buffer;

    uint64 n_samples = static_cast<uint64>(std::ceil(duration * sample_rate_));

    for(uint64 i = 0; i < n_samples; ++i)
    {
        buffer << generate2(frequency, phase);
    }

    return buffer;
}

Buffer
Generator::
generate(
    const float64 & duration,
    const Buffer &  frequencies)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    if(!is_realtime_) reset();

    uint32 n_samples = static_cast<uint32>(std::ceil(duration * sample_rate_));

    Buffer y(n_samples);

    Buffer::const_circular_iterator freq = frequencies.cbegin();

    for(uint32 i = 0; i < n_samples; ++i, ++freq)
    {
        y << generate(*freq);
    }

    return y;
}

Buffer
Generator::
generate2(
    const float64 & duration,
    const float64 & frequency,
    const Buffer &  phase)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    if(!is_realtime_) reset();

    uint32 n_samples = static_cast<uint32>(std::ceil(duration * sample_rate_));

    Buffer y(n_samples);

    Buffer::const_circular_iterator p = phase.cbegin();

    for(uint32 i = 0; i < n_samples; ++i, ++p)
    {
        y << generate2(frequency,*p);
    }

    return y;
}

Buffer
Generator::
generate2(
    const float64 & duration,
    const Buffer &  frequencies,
    const float64 & phase)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    if(!is_realtime_) reset();

    uint32 n_samples = static_cast<uint32>(std::ceil(duration * sample_rate_));

    Buffer y(n_samples);

    Buffer::const_circular_iterator f = frequencies.cbegin();

    for(uint32 i = 0; i < n_samples; ++i, ++f)
    {
        y << generate2(*f, phase);
    }

    return y;
}

Buffer
Generator::
generate2(
    const float64 & duration,
    const Buffer &  frequencies,
    const Buffer &  phase)
{
    M_ASSERT_VALUE(duration, >, 0.0);

    if(!is_realtime_) reset();

    uint32 n_samples = static_cast<uint32>(std::ceil(duration * sample_rate_));

    Buffer y(n_samples);

    Buffer::const_circular_iterator f = frequencies.cbegin();
    Buffer::const_circular_iterator p = phase.cbegin();

    for(uint32 i = 0; i < n_samples; ++i, ++f, ++p)
    {
        y << generate2(*f,*p);
    }

    return y;
}

Generator &
Generator::
operator=(const Generator & rhs)
{
    // If this is the same object, return it.
    if(this == &rhs)
    {
        return *this;
    }

    last_frequency_ = rhs.last_frequency_;
    position_       = rhs.position_;
    sample_rate_    = rhs.sample_rate_;
    sample_time_    = rhs.sample_time_;
    t_              = rhs.t_;

    if(waveform_ != NULL && rhs.waveform_ != NULL)
    {
        *waveform_ = *rhs.waveform_;
    }
    else if(waveform_ == NULL && rhs.waveform_ != NULL)
    {
        waveform_ = new Buffer(*rhs.waveform_);
    }
    else if(waveform_ != NULL && rhs.waveform_ == NULL)
    {
        delete waveform_;
        waveform_ = NULL;
    }

    *rng_ = *rhs.rng_;

    buzz_max_harmonics_ = rhs.buzz_max_harmonics_;
    buzz_position_      = rhs.buzz_position_;

    chorus_is_on_    = rhs.chorus_is_on_;
    chorus_n_voices_ = rhs.chorus_n_voices_;
    chorus_position_ = rhs.chorus_position_;
    chorus_factor_   = rhs.chorus_factor_;

    sync_is_master_ = rhs.sync_is_master_;
    sync_is_slave_  = rhs.sync_is_slave_;
    sync_count_     = rhs.sync_count_;
    sync_vector_    = rhs.sync_vector_;
    sync_slaves_    = rhs.sync_slaves_;

    return *this;
}

void
Generator::
removeSlaveSync(Generator & slave)
{
    slave.sync_is_slave_ = false;

    if(sync_slaves_.count(&slave) == 0) return;

    sync_slaves_.erase(&slave);

    reset();
}

void
Generator::
reset()
{
    last_frequency_ = -1.0;
    position_ = 0.0;
    sync_count_ = 0;
    sync_pos_ = 0.0;
    t_ = 0.0;

    buzz_position_.reserve(buzz_max_harmonics_);

    for(uint32 i = 0; i < buzz_max_harmonics_; ++i)
    {
        buzz_position_[i] = 0.0;
    }

    for(uint32 i = 0; i < chorus_n_voices_; ++i)
    {
        chorus_position_[i] = 0.0;
    }

    std::set<Generator *>::iterator itor = sync_slaves_.begin();

    while(itor != sync_slaves_.end())
    {
        (*itor)->sync_vector_.clear();
        ++itor;
    }
}

void
Generator::
setSeed(const uint32 seed)
{
    rng_->setSeed(seed);
}

Buffer
Generator::
silence(const float64 & duration) const
{
    return drawLine(duration,0.0,0.0);
}

float64
Generator::
tell() const
{
    return position_ / sample_rate_;
}


Buffer
Generator::
whiteNoise(const float64 & duration) const
{
    M_ASSERT_VALUE(duration, >, 0.0);

    Buffer buffer;

    uint64 n_samples = static_cast<uint64>(std::ceil(duration * sample_rate_));

    for(uint64 i = 0; i < n_samples; ++i)
    {
        buffer << rng_->get(-1.0f,1.0f);
    }

    return buffer;
}

Buffer
Generator::
gaussianNoise(
    const float64 & duration,
    const float64 & mu,
    const float64 & sigma) const
{
    Buffer buffer;

    uint32 n_samples = static_cast<uint32>(std::ceil(duration * sample_rate_));

    M_ASSERT_VALUE(n_samples, >, 0);

    // The polar form of the Box-Muller transformation.

    uint32 i = 0;
    while(true)
    {
        float64 f = 0.0;
        float64 x1 = 0.0;
        float64 x2 = 0.0;
        float64 r2 = 0.0;

        while(r2 == 0.0 || r2 >= 1.0)
        {
            x1 = rng_->get(-1.0f, 1.0f);
            x2 = rng_->get(-1.0f, 1.0f);
            r2 = x1 * x1 + x2 * x2;
        }

        f = ::sqrt(-2.0 * ::log(r2) / r2);

        buffer << mu + sigma * f * x1;

        ++i;

        if(i >= n_samples)
        {
            break;
        }

        buffer << mu + sigma * f * x2;

        ++i;

        if(i >= n_samples)
        {
            break;
        }
    }

    return buffer;
}

Buffer
Generator::
tanh(const float64 & duration) const
{
    M_ASSERT_VALUE(duration, >, 0.0);

    Buffer buffer;

    uint64 n_samples = static_cast<uint64>(std::ceil(duration * sample_rate_));

    float64 step = (2.0 * M_PI) / n_samples;

    float64 x = -1.0 * M_PI;

    for(uint64 i = 0; i < n_samples; ++i)
    {
        buffer << std::tanh(x);
        x += step;
    }

    return buffer;
}
