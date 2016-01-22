//-----------------------------------------------------------------------------
//
//  $Id: Granulator.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//  Nsound is a C++ library and Python module for audio synthesis featuring
//  dynamic digital filters. Nsound lets you easily shape waveforms and write
//  to disk or plot them. Nsound aims to be as powerful as Csound but easy to
//  use.
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

#include <Nsound/Nsound.h>
#include <Nsound/Buffer.h>
#include <Nsound/Generator.h>
#include <Nsound/Granulator.h>
#include <Nsound/Sine.h>

#include <iostream>

using namespace Nsound;

using std::cerr;
using std::endl;

///////////////////////////////////////////////////////////////////////////
Granulator::
Granulator(
    const float64  & sample_rate,
    EnvelopeType_e   env_type,
    const float64  & envelope_noise,
    const Buffer   * custom_envelope)
    :
    sample_rate_(sample_rate),
    envelope_generator_(NULL)
//     sine_(new Sine(sample_rate_)),
//     current_time_(0.0),
//     next_grain_time_(0.0),
//     last_grain_time_(0.0),
//     time_step_(1.0 / sample_rate_)
{
    Generator gen(sample_rate_);

    switch(env_type)
    {
        case Granulator::CUSTOM:
        {
            if(custom_envelope == NULL
                || custom_envelope->getLength() != sample_rate_)
            {
                M_THROW("custom_envelope->getLength() must equal "
                     << sample_rate_
                     << ", length is "
                     << custom_envelope->getLength());

                envelope_generator_ =
                    new Generator(
                        sample_rate_,
                        gen.drawGaussian(1.0, 0.5, 0.33333)
                          * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            }
            else
            {
                envelope_generator_ =
                    new Generator(
                        sample_rate_,
                        (*custom_envelope)
                            * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            }
            break;
        }

        case Granulator::GAUSSIAN:
        {
            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawGaussian(1.0, 0.5, 0.33333)
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }

        case Granulator::GAUSSIAN_90:
        {
            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawFatGaussian(1.0, 0.90)
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }

        case Granulator::GAUSSIAN_70:
        {
            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawFatGaussian(1.0, 0.70)
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }

        case Granulator::GAUSSIAN_50:
        {
            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawFatGaussian(1.0, 0.50)
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }

        case Granulator::GAUSSIAN_30:
        {
            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawFatGaussian(1.0, 0.30)
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }

        case Granulator::GAUSSIAN_10:
        {
            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawFatGaussian(1.0, 0.10)
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }

        case Granulator::DECAY:
        {
            Buffer waveform(static_cast<uint32>(sample_rate_));

            waveform << gen.drawDecay(1.0);

            Buffer noise = 1.0 + envelope_noise * gen.whiteNoise(1.0);

            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawDecay(1.0)
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }

        case Granulator::REVERSE_DECAY:
        {
            envelope_generator_ = new Generator(
                sample_rate_,
                gen.drawDecay(1.0).getReverse()
                    * (1.0 + envelope_noise * gen.whiteNoise(1.0)));
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
Granulator::
Granulator(const Nsound::Granulator & gran)
    :
    sample_rate_(gran.sample_rate_),
    envelope_generator_(NULL)
{
    *this = gran;
}

///////////////////////////////////////////////////////////////////////////
Granulator::
~Granulator()
{
    delete envelope_generator_;
}

Buffer
Granulator::
generate(
    const float64 & duration,
    const float64 & grain_frequency,
    const float64 & waves_per_grain,
    const float64 & grains_per_second)
{
    Sine sin(sample_rate_);

    float64 time_step = 1.0 / sample_rate_;

    Buffer y(4096);

    for(float64 last_g_time = 0.0, current_time = 0.0, next_grain_time = 0.0;
        current_time < duration;
        current_time += time_step)
    {
        // Output a pulse.
        if(next_grain_time - current_time < time_step)
        {
            float64 g_duration  = waves_per_grain / grain_frequency;

            Buffer grain(4096);

            grain <<  sin.generate(g_duration, grain_frequency)
                    * envelope_generator_->generate(g_duration, 1.0 / g_duration);

            y.add(grain, static_cast<uint32>(current_time * sample_rate_));

            last_g_time = current_time;
        }

        next_grain_time = last_g_time + (1.0 / grains_per_second);
    }

    return y;
}

Buffer
Granulator::
generate(
    const float64 & duration,
    const Buffer & grain_frequency,
    const Buffer & waves_per_grain,
    const Buffer & grains_per_second) const
{
    Sine sin(sample_rate_);

    uint32 gf_length  = grain_frequency.getLength();
    uint32 wpg_length = waves_per_grain.getLength();
    uint32 gps_length = grains_per_second.getLength();

    uint32 gf_index = 0;
    uint32 wpg_index = 0;
    uint32 gps_index = 0;

    float64 time_step = 1.0 / sample_rate_;

    Buffer y;

    for(float64 last_g_time = 0.0, current_time = 0.0, next_grain_time = 0.0;
        current_time < duration;
        current_time += time_step)
    {

        float64 gps =
            grains_per_second[gps_index++ % gps_length];

        // Output a pulse.
        if(next_grain_time - current_time < time_step)
        {
            float64 g_frequency = grain_frequency[gf_index++  % gf_length];
            float64 wpg         = waves_per_grain[wpg_index++ % wpg_length];

            float64 g_duration  = wpg / g_frequency;

            Buffer grain =
                sin.generate(g_duration,g_frequency)
                * envelope_generator_->generate(g_duration, 1.0 / g_duration);

            y.add(grain, static_cast<uint32>(current_time * sample_rate_));

            last_g_time = current_time;
        }

        next_grain_time = last_g_time + (1.0 / gps);
    }

    return y;
}


///////////////////////////////////////////////////////////////////////////
Granulator &
Granulator::
operator=(const Granulator & rhs)
{
    delete envelope_generator_;

    sample_rate_ = rhs.sample_rate_;
    envelope_generator_ = new Generator(*rhs.envelope_generator_);

    return *this;
}


