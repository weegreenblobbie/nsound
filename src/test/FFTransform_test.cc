//-----------------------------------------------------------------------------
//
//  FFTransform and FFTChunk tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/Cosine.h>
#include <Nsound/FFTChunk.h>
#include <Nsound/FFTransform.h>
#include <Nsound/Sine.h>
#include <Nsound/Wavefile.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

static const float64 GAMMA = 1.0e-11;

TEST_CASE("FFTransform")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    Cosine cos(512);

    Buffer input = cos.buzz(1.0, 8.0, 2.0, 0.0);

    FFTransform fft(512);

    SECTION("fft(), ifft()")
    {
        FFTChunkVector vec = fft.fft(input, 128, 0);

        REQUIRE(vec.size() == 512 / 128);

        Buffer data = fft.ifft(vec);

        REQUIRE(data.getLength() == input.getLength());

        check_gold(data, "gold/FFTransform_out1.wav", GAMMA);
    }

    SECTION("FFTChunk getReal(), getImaginary()")
    {
        FFTChunkVector vec = fft.fft(input, 128, 0);

        check_gold(vec[0].getReal(), "gold/FFTransform_out2.wav", GAMMA);
        check_gold(vec[0].getImaginary(), "gold/FFTransform_out3.wav", GAMMA);
    }

    SECTION("FFTChunk getMagnitude(), getPhase()")
    {
        FFTChunkVector vec = fft.fft(input, 128, 0);

        Buffer mag = vec[0].getMagnitude();

        check_gold(mag, "gold/FFTransform_out4.wav", GAMMA);

        // The phase of a bin with no energy (magnitude ~1e-14 here, the
        // peak is 32) is rounding noise, so only bins with energy are
        // compared.  Phases that differ by about 2 pi are the same angle.
        Buffer phase = vec[0].getPhase();
        Buffer gold("gold/FFTransform_out5.wav");

        REQUIRE(phase.getLength() == gold.getLength());

        Buffer abs_diff = (phase - gold).getAbs();
        abs_diff(mag < 1e-9) = 0.0;
        abs_diff(abs_diff > 2.0 * M_PI - 0.01) = 0.0;

        CHECK(abs_diff.getMax() <= GAMMA);
    }

    SECTION("FFTChunk toPolar()")
    {
        FFTChunkVector vec = fft.fft(input, 128, 0);

        vec[0].toPolar();

        check_gold(vec[0].getMagnitude(), "gold/FFTransform_out4.wav", GAMMA);

        // Only checks the phase is within one turn of the gold phase.
        check_gold(vec[0].getPhase(), "gold/FFTransform_out5.wav", 2 * M_PI);
    }

    SECTION("FFTChunk setPolar(), setCartesian(), getFrequencyAxis()")
    {
        FFTChunkVector vec = fft.fft(input, 512, 0);

        Buffer mag = vec[0].getMagnitude();
        Buffer phase = vec[0].getPhase();

        FFTChunk orig(vec[0]);
        FFTChunk chunk(512, 512);

        chunk.setPolar(mag, phase);
        vec[0] = chunk;

        check_close(fft.ifft(vec), input, 0.005);

        chunk.setCartesian(orig.getReal(), orig.getImaginary());
        vec[0] = chunk;

        check_close(fft.ifft(vec), input, 2 * GAMMA);

        check_gold(vec[0].getFrequencyAxis(), "gold/FFTransform_out6.wav", 1e-16);
    }
}

TEST_CASE("FFTChunk phase of empty bins")
{
    // Most bins of this spectrum are exactly 0 + 0i, which getPhase() and
    // toPolar() turned into NaN.
    Cosine cos(512);

    FFTransform fft(512);

    FFTChunkVector vec = fft.fft(cos.buzz(1.0, 8.0, 2.0, 0.0), 512, 0);

    Buffer phase = vec[0].getPhase();

    vec[0].toPolar();

    Buffer polar_phase = vec[0].getPhase();

    for(uint32 i = 0; i < phase.getLength(); ++i)
    {
        INFO("bin " << i);
        CHECK_FALSE(std::isnan(phase[i]));
        CHECK_FALSE(std::isnan(polar_phase[i]));
    }
}

TEST_CASE("FFTChunk setPolar() keeps the phase")
{
    // A sine's bins have phase -pi/2, setPolar() used to double it.
    Sine sine(512);

    Buffer input = sine.generate(1.0, 8.0);

    FFTransform fft(512);

    FFTChunkVector vec = fft.fft(input, 512, 0);

    FFTChunk chunk(512, 512);

    chunk.setPolar(vec[0].getMagnitude(), vec[0].getPhase());

    vec[0] = chunk;

    check_close(fft.ifft(vec), input, 2 * GAMMA);
}

TEST_CASE("FFTransform fft() and ifft() of a Buffer")
{
    FFTransform t(1000.0);
    Sine sine(1000.0);

    SECTION("fft() is the magnitude at 1 Hz a sample up to Nyquist")
    {
        Buffer m = t.fft(sine.generate(1.0, 125.0));

        REQUIRE(m.getLength() == 500);

        uint32 peak = 0;
        for(uint32 i = 1; i < m.getLength(); ++i) if(m[i] > m[peak]) peak = i;

        CHECK(peak == 125);
    }

    SECTION("ifft() of one bin is a cosine")
    {
        // Bin 4 of a mirrored 128 point spectrum, 2 cycles in 64 samples.
        Buffer spectrum = Buffer::zeros(64);
        spectrum[4] = 1.0;

        Buffer y = t.ifft(spectrum);

        REQUIRE(y.getLength() == 64);

        uint32 crossings = 0;
        for(uint32 i = 1; i < y.getLength(); ++i)
        {
            if((y[i - 1] < 0.0) != (y[i] < 0.0)) ++crossings;
        }

        CHECK(crossings == 4);
        CHECK(y[0] > 0.0);
    }

    SECTION("setWindow() windows the chunks")
    {
        Buffer x = sine.generate(0.5, 125.0);

        Buffer plain = t.fft(x, 256, 0)[0].getMagnitude();

        t.setWindow(HANNING);

        Buffer windowed = t.fft(x, 256, 0)[0].getMagnitude();

        CHECK((windowed - plain).getAbs().getMax() > 1.0);
    }
}
