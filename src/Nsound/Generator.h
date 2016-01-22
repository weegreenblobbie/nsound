//-----------------------------------------------------------------------------
//
//  $Id: Generator.h 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
//
//  Copyright (c) 2005-Present Nick Hilton
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

#ifndef _NSOUND_GENERATOR_H_
#define _NSOUND_GENERATOR_H_

#include <Nsound/Nsound.h>
#include <Nsound/WindowType.h>

namespace Nsound
{

// Forwared declarion of Buffer
class Buffer;
class RandomNumberGenerator;

//-----------------------------------------------------------------------------
//  Generator Class
//! A class the provides draw utilities and a wavetable oscillator.
class Generator
{
    public:

    //! Creates a generator with the specified sample rate.
    Generator(const float64 & sample_rate);

    //! Creates a generator with the specified sample rate and waveform.
    Generator(const float64 & sample_rate, const Buffer & waveform);

    //! Creates a generator using the waveform stored in wave_filename.
    //
    //! The sample rate of the generator will be the length of the waveform.
    Generator(const std::string & wave_filename);

    //! Copy constructor
    Generator(const Nsound::Generator & gen);

    //! Deletes the baseclass part of the object.
    virtual ~Generator();

    //! Sets realtime mode, disables automatic reset() if set.
    void setRealtime(bool flag) { is_realtime_ = flag; }

    //! Adds a generator as a slave to this instance for syncing.
    void
    addSlaveSync(Generator & slave);

    //! Initaializes Buzz settings for real-time calls.
    void
    buzzInit(const uint32 & max_harmonics);

    //! Returns sample from a set of harmonics.  Based on the Csound buzz opcode.
    //
    //! Chorus can not be used with this function.
    float64
    buzz(
        const float64 & frequency,
        const float64 & n_harmonics,
        const float64 & delay);

    //! Returns a set of harmonics.  Based on the Csound buzz opcode.
    Buffer
    buzz(
        const float64 & duration,
        const float64 & frequency,
        const float64 & n_harmonics,
        const float64 & delay);

    //! Returns a set of harmonics.  Based on the Csound buzz opcode.
    Buffer
    buzz(
        const float64 & duration,
        const Buffer & frequency,
        const Buffer & n_harmonics,
        const Buffer & delay);

    //! Chorus or Unison
    //
    //! When chorus is not enabled, samples are generated using one frequency
    //! indexing into the wave table.  However, when chorus is turned on, the
    //! output sample will be the average of N frequencies indexing into the
    //! wavetable.  A set of scalar is randomally selected, each frequncy in
    //! the set is calculated by
    //! \f[ f_n = \left({1.0 + \alpha_n}\right) f \f]
    //! where \f$\alpha_n\f$ is a random scalar selected inside \f$\pm\sigma\f$.
    //!
    void
    setChorus(
        const uint32 n_voices,
        const float64 & sigma = 0.02);

    //! This method draws an exponential curve that decays from 1.0 to 0.0 over the duration.
    //
    //! \f[ y\left[n\right]=e^{-\alpha n\tau} \f]
    //!
    //! where the default value for \f$\alpha\f$ is \f$2\pi\f$ and \f$\tau\f$ is the sample time.
    Buffer
    drawDecay(const float64 & duration, const float64 & alpha = 2.0*M_PI) const;

    //! This method draws a Gaussian curve over duration seconds.
    //
    //! This method draws a Gaussian curve over duration seconds.
    //! \param duration the number of seconds to draw
    //! \param mu \f$\left(\mu\right)\f$ the location of the peak in seconds
    //! \param sigma \f$\left(\sigma\right)\f$ the width of the Gaussian in seconds
    //! \param normalize a flag to normalize the Gaussian so the peak is 1.0
    //! \return Buffer
    //!
    //! The generatl equation for a discrete Gaussian is given by
    //!
    //! \f[ y\left[n\right]=\frac{1}{\sqrt{2\pi\sigma^{2}}}e^{-\frac{\left(n\tau-\mu\right)^{2}}{2\sigma^{2}}} \f]
    //!
    //! where \f$\tau\f$ is the sample time.
    //!
    Buffer
    drawGaussian(
        const float64 & duration,
        const float64 & mu,
        const float64 & sigma,
        const boolean & normalize = true) const;

    //! This method draws a standard Gaussian curve over duration seconds, with a specified pass band.
    //
    //! \param duration the number of seconds to draw
    //! \param pass_band_percent the width of the pass band in percent of duraiton
    //! \param sigma the standard deviation of the gaussian curve
    //! \return Buffer
    //!
    //! The curve is always normaized so the peak is 1.0.
    Buffer
    drawFatGaussian(
        const float64 & duration,
        const float64 & pass_band_percent = 0.01) const;

    //! This method draws a linear line beteween 2 points.
    //
    //! \param duration the number of seconds to draw
    //! \param amplitude_start the starting amplitude value
    //! \param amplitude_finish the finishing amplitude value
    //! \return Buffer
    //!
    //! The two points are located at (x1,y1) = (0.0, amplitude_1) and
    //! (x2,y2) = (duration, amplitude_2).
    Buffer
    drawLine(
        const float64 & duration,
        const float64 & amplitude_start,
        const float64 & amplitude_finish) const;

    //! This method draws a parabola between three points, intersecting the middle point.
    //
    //! \param duration the number of seconds to draw
    //! \param y1 the y1 amplitude
    //! \param x2 the x2 time
    //! \param y2 the y2 amplitude
    //! \param y3 the y3 amplitude
    //! \return Buffer
    //!
    //! The general equation for a discrete parabola is:
    //!
    //! \f[ y\left[n\right]=a\left(n\tau\right)^{2}+b\left(n\tau\right)+c \f]
    //!
    //! where \f$\tau\f$ is the sample time.  The three points are located at
    //! (x1,y1), (x2,y2), (x3,y3) with x1 = 0.0 and x3 = duration.
    //!
    Buffer
    drawParabola(
        const float64 & duration,
        const float64 & y1,
        const float64 & x2,
        const float64 & y2,
        const float64 & y3) const;

    //! This method draws a static sine wave.
    //
    //! \param duration the number of seconds to draw
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return Buffer
    //!
    //! This is a special case of
    //! drawSine2(const float64 & duration, const float64 & frequency, const float64 & phase)
    //! with phase set to 0.0.
    Buffer
    drawSine(
        const float64 & duration,
        const float64 & frequency);

    //! This method draws a dynamic sine wave.
    //
    //! \param duration the number of seconds to draw
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return Buffer
    //!
    //! This is a special case of
    //! drawSine2(const float64 & duration, const Buffer & frequency, const Buffer & phase)
    //! with the phase set to 0.0.
    Buffer
    drawSine(
        const float64 & duration,
        const Buffer & frequency);

    //! This method draws a static sine wave.
    //
    //! \param duration the number of seconds to draw
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    //!
    //! The general discrete sine wave is given by
    //! \f[ y\left[n\right]=\sin\left(2\pi fn\tau+\pi\varphi\right) \f]
    //! where \f$\tau\f$ is the sample time. This function will produce a
    //! cosine wave if \f$\varphi\f$ is 0.5.
    Buffer
    drawSine2(
        const float64 & duration,
        const float64 & frequency,
        const float64 & phase);

    //! This method draws a dynamic sine wave.
    //
    //! \param duration the number of seconds to draw
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    //!
    //! The general dynamic discrete sine wave is given by
    //! \f[ y\left[n\right]=\sin\left(2\pi s\left[n\right]\tau+\pi\varphi_{n}\right) \f]
    //! where \f$s\left[n\right]\f$ is given by
    //! \f[ s\left[n\right]=\sum_{i=0}^{n}f_{n} \f]
    //! and \f$s\left[n\right]=0\f$ when \f$n=0\f$ and \f$\tau\f$ is the sample time.
    //!
    //! This function will produce a cosine wave if \f$\varphi_n\f$ is 0.5.
    Buffer
    drawSine2(
        const float64 & duration,
        const Buffer & frequency,
        const Buffer & phase);

    //! This method draws a dynamic sine wave.
    //
    //! \param duration the number of seconds to draw
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    //!
    //! This is a special case of
    //! drawSine2(const float64 & duration, const Buffer & frequency, const Buffer & phase)
    //! with the phase set to a constant value.
    Buffer
    drawSine2(
        const float64 & duration,
        const Buffer & frequency,
        const float64 & phase=0.0);

    //! This method draws a dynamic sine wave.
    //
    //! \param duration the number of seconds to draw
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    //!
    //! This is a special case of
    //! drawSine2(const float64 & duration, const Buffer & frequency, const Buffer & phase)
    //! with the frequency set to a constant value.
    Buffer
    drawSine2(
        const float64 & duration,
        const float64 & frequency,
        const Buffer & phase);

    //! This method draws one sample of a sine wave in real-time.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return float64
    //!
    //! This is a special case of
    //! drawSine2(const float64 & frequency, const float64 & phase)
    //! with the phase = 0.0.
    float64
    drawSine(
        const float64 & frequency);

    //! This method draws one sample of a sine wave in real-time.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return float64
    float64
    drawSine2(
        const float64 & frequency,
        const float64 & phase);

    //! Draws a window of the specified type.
    Buffer
    drawWindow(const float64 & duration, WindowType type) const;

    //! Draws a Bartlett window.
    //
    Buffer
    drawWindowBartlett(const float64 & duration) const;

    //! Draws a Blackman window.
    //
    //! The general discrete cosine window is given by
    //! \f[ w\left[n\right]=a_{0}-a_{1}\cos\left(2\pi n\tau\right)+a_{2}\cos\left(4\pi n\tau\right)-a_{3}\cos\left(6\pi n\tau\right) \f]
    //! where \f$\tau\f$ is the sample time.  The Blackman window is a special
    //! case of the cosesine window with
    //! \f$a_0\f$ = 0.42,
    //! \f$a_1\f$ = 0.50,
    //! \f$a_2\f$ = 0.08,
    //! \f$a_3\f$ = 0.00.
    Buffer
    drawWindowBlackman(const float64 & duration) const;

    //! Draws a Blackman-Harris window.
    //
    //! The general discrete cosine window is given by
    //! \f[ w\left[n\right]=a_{0}-a_{1}\cos\left(2\pi n\tau\right)+a_{2}\cos\left(4\pi n\tau\right)-a_{3}\cos\left(6\pi n\tau\right) \f]
    //! where \f$\tau\f$ is the sample time.  The Blackman-Harris window is a
    //! special case of the cosesine window with
    //! \f$a_0\f$ = 0.35875,
    //! \f$a_1\f$ = 0.48829,
    //! \f$a_2\f$ = 0.14128,
    //! \f$a_3\f$ = 0.01168.
    Buffer
    drawWindowBlackmanHarris(const float64 & duration) const;

    //! Draws a Hamming window.
    //
    //! The general discrete cosine window is given by
    //! \f[ w\left[n\right]=a_{0}-a_{1}\cos\left(2\pi n\tau\right)+a_{2}\cos\left(4\pi n\tau\right)-a_{3}\cos\left(6\pi n\tau\right) \f]
    //! where \f$\tau\f$ is the sample time.  The Hamming window is a
    //! special case of the cosesine window with
    //! \f$a_0\f$ = 0.54,
    //! \f$a_1\f$ = 0.46,
    //! \f$a_2\f$ = 0.00,
    //! \f$a_3\f$ = 0.00.
    Buffer
    drawWindowHamming(const float64 & duration) const;

    //! Draws a Hanning window.
    //
    //! The general discrete cosine window is given by
    //! \f[ w\left[n\right]=a_{0}-a_{1}\cos\left(2\pi n\tau\right)+a_{2}\cos\left(4\pi n\tau\right)-a_{3}\cos\left(6\pi n\tau\right) \f]
    //! where \f$\tau\f$ is the sample time.  The Hanning window is a
    //! special case of the cosesine window with
    //! \f$a_0\f$ = 0.50,
    //! \f$a_1\f$ = 0.50,
    //! \f$a_2\f$ = 0.00,
    //! \f$a_3\f$ = 0.00.
    Buffer
    drawWindowHanning(const float64 & duration) const;

    //! Draws a Kaiser window.
    Buffer
    drawWindowKaiser(const float64 & duration, const float64 & beta=5.0) const;

    //! Draws a Nuttall window.
    //
    //! The general discrete cosine window is given by
    //! \f[ w\left[n\right]=a_{0}-a_{1}\cos\left(2\pi n\tau\right)+a_{2}\cos\left(4\pi n\tau\right)-a_{3}\cos\left(6\pi n\tau\right) \f]
    //! where \f$\tau\f$ is the sample time.  The Nuttall window is a
    //! special case of the cosesine window with
    //! \f$a_0\f$ = 0.3635819,
    //! \f$a_1\f$ = 0.4891775,
    //! \f$a_2\f$ = 0.1365995,
    //! \f$a_3\f$ = 0.0106411.
    Buffer
    drawWindowNuttall(const float64 & duration) const;

    //! Draws a Parzen window.
    Buffer
    drawWindowParzen(const float64 & duration) const;

    //! Draws a rectangular window.
    Buffer
    drawWindowRectangular(const float64 & duration) const;

    //! This is a real-time method for the wavetable oscillator.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return float64
    //!
    //! This is a special case of
    //! generate2(const float64 & frequency, const float64 & phase)
    //! with phase = 0.0.
    virtual
    float64
    generate(const float64 & frequency);

    //! This is a real-time method for the wavetable oscillator.
    //
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return float64
    //!
    //! The general wavetable oscillator equation is given by
    //! \f[ y\left[n\right]=wavetable\left[s\left[n\right]\right] \f]
    //!
    //! where the index into the table, \f$s\left[n\right]\f$ is given by the
    //! recursive function
    //! \f[ s\left[n\right]=\left(s\left[n-1\right]+f+\pi\varphi\right)\div N \f]
    //!
    //! where \f$\div\f$ is the modulus operator, \f$N\f$ is the wavetable
    //! length, when \f$n<0\f$, \f$s\left[n\right]=0\f$.
    //! In Nsound, the wavetable size, \f$N\f$, is equal to the sample rate.
    //!
    virtual
    float64
    generate2(const float64 & frequency, const float64 & phase);

    //! This method oscillates the waveform stored in this generator.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return Buffer
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Generator gen("some_waveform.wav");
    //! Buffer b = gen.generate(1.0, 3.0);
    //!
    //! // Python
    //! gen = Generator("some_waveform.wav")
    //! b = gen.generate(1.0, 3.0)
    //! \endcode
    virtual
    Buffer
    generate(
        const float64 & duration,
        const float64 & frequency);

    //! This method oscillates the waveform stored in this generator.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    //!
    //! \par Example:
    //! \code
    //! // C++
    //! Generator gen("some_waveform.wav");
    //! Buffer b = gen.generate2(1.0, 3.0, 0.5); // 90 degrees out of phase
    //!
    //! // Python
    //! gen = Generator("some_waveform.wav")
    //! b = gen.generate(1.0, 3.0, 0.5)
    //! \endcode
    virtual
    Buffer
    generate2(
        const float64 & duration,
        const float64 & frequency,
        const float64 & phase);

    //! This method oscillates the waveform stored in this generator.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \return Buffer
    //!
    //! This is a special case of
    //! generate2(const float64 & duration, const Buffer & frequencies, const Buffer & phase)
    //! with phase = 0.0.
    virtual
    Buffer
    generate(
        const float64 & duration,
        const Buffer & frequencies);

    //! This method oscillates the waveform stored in this generator.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    //!
    //! This is a special case of
    //! generate2(const float64 & duration, const Buffer & frequencies, const Buffer & phase)
    //! with constant frequency.
    virtual
    Buffer
    generate2(
        const float64 & duration,
        const float64 & frequencies,
        const Buffer &  phase);

    //! This method oscillates the waveform stored in this generator.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    //!
    //! This is a special case of
    //! generate2(const float64 & duration, const Buffer & frequencies, const Buffer &  phase)
    //! with constant phase.
    virtual
    Buffer
    generate2(
        const float64 & duration,
        const Buffer &  frequencies,
        const float64 & phase);

    //! This method oscillates the waveform stored in this generator.
    //
    //! \param duration the number of seconds to oscillate the waveform
    //! \param frequency \f$\left(f\right)\f$ the frequency in Hz
    //! \param phase \f$\left(\varphi\right)\f$ the natural phase (0.0 to 1.0)
    //! \return Buffer
    virtual
    Buffer
    generate2(
        const float64 & duration,
        const Buffer &  frequencies,
        const Buffer &  phase);

    //! Assignment operator.
    Nsound::Generator &
    operator=(const Nsound::Generator & rhs);

    //! Removes the generator from the sync list.
    void
    removeSlaveSync(Generator & slave);

    //! Resets the position pointer back to the begging of the waveform.
    virtual
    void reset();

    RandomNumberGenerator & getRandomNumberGenerator() {return *rng_; }

    //! Sets the seed for the Generator's random number generator (rng).
    void setSeed(const uint32 seed);

    //! This method generates silence.
    //
    //! This is a special coase of
    //! drawLine(const float64 & duration, const float64 & amplitude_start, const float64 & amplitude_finish) const
    //! where the start and stopping amplitudes are 0.0.
    Buffer
    silence(const float64 & duration) const;

    //! Returns the current position pointer, values are between 0.0 to 1.0.
    float64
    tell() const;

    //! This method generates noise from a uniform distribution.
    Buffer
    whiteNoise(const float64 & duration) const;

    //! This method generates noise from a Gaussian distribution.
    Buffer
    gaussianNoise(
        const float64 & duration,
        const float64 & mu,
        const float64 & sigma) const;

    //! This method draws the tanh function accross duration samples.
    Buffer
    tanh(const float64 & duration) const;

    protected:

    //! DOXME
    Generator();

    //! DOXME
    virtual
    void
    ctor(const float64 & sample_rate);

    //! DOXME
    virtual
    void
    ctor(
        const float64 & sample_rate,
        const Buffer & wavetable);

    bool is_realtime_;

    float64  last_frequency_;  //! Used for phase offset adjustment.
    float64  position_;        //! The number of samples into the wavefrom
    float64  sync_pos_;        //! Used to determine when to create a sync sample.
    float64  sample_rate_;     //! The number of samples per second to generate.
    float64  sample_time_;     //! The time step between samples in seconds.
    float64  t_;               //! The current time (for real time draw functions.)
    Buffer * waveform_;        //! The waveform to ossicialate.

    RandomNumberGenerator * rng_; //! The random number generator.

    // buzz() stuff
    uint32               buzz_max_harmonics_;
    std::vector<float64> buzz_position_;

    // Chorus stuff
    boolean              chorus_is_on_;
    uint32               chorus_n_voices_;
    std::vector<float64> chorus_position_;
    std::vector<float64> chorus_factor_;

    // Sync stuff
    boolean sync_is_master_;               //! Indicates if this generator is the master
    boolean sync_is_slave_;                //! Indicates if this generator is a slave
    uint32 sync_count_;                    //! Indicates the number of samples since reset
    std::vector<uint32> sync_vector_;      //! Stores sample counts when synced
    std::set<Generator *> sync_slaves_;    //! Holds pointers to the slaves.

};//Generators

}; // Nsound

// :mode=c++: jEdit modeline
#endif
