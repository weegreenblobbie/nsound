//-----------------------------------------------------------------------------
//
//  FilterLeastSquaresFIR tests, compared with the gold files.
//
//-----------------------------------------------------------------------------

// FilterLeastSquaresFIR.h needs AudioStream.h first.
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/FilterLeastSquaresFIR.h>
#include <Nsound/Wavefile.h>

#include "filter_signals.h"
#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

static const float64 GAMMA = 1.5e-14;

TEST_CASE("FilterLeastSquaresFIR")
{
    Wavefile::setDefaultSampleSize(64);
    Wavefile::setIEEEFloat(true);

    float64 sr = 1000.0;

    // A low pass least squares FIR filter.
    Buffer freqs;
    Buffer amplitude;

    freqs     << 0.0 << 100.0 << 100.0 << 500.0;
    amplitude << 1.0 << 1.0   << 0.0   << 0.0;

    FilterLeastSquaresFIR f(sr, 1025, freqs, amplitude);

    Buffer noise("gold/Filter_noise.wav");

    SECTION("low pass filter(input)")
    {
        check_gold(f.filter(noise), "gold/FilterLeastSquaresFIR_out1.wav", GAMMA);

        // Repeated to test that reset() is being called.
        check_gold(f.filter(noise), "gold/FilterLeastSquaresFIR_out1.wav", GAMMA);
    }

    SECTION("high pass makeKernel(), filter(input)")
    {
        freqs = Buffer(4);
        amplitude = Buffer(4);

        freqs     << 0.0 << 100.0 << 100.0 << 500.0;
        amplitude << 0.0 << 0.0   << 1.0   << 1.0;

        f.makeKernel(freqs, amplitude);

        check_gold(f.filter(0.9 * noise), "gold/FilterLeastSquaresFIR_out2.wav", GAMMA);

        // Repeated to test that reset() is being called.
        check_gold(f.filter(0.9 * noise), "gold/FilterLeastSquaresFIR_out2.wav", GAMMA);
    }

    SECTION("setWindow()")
    {
        freqs     = Buffer(4);
        amplitude = Buffer(4);

        freqs     << 0.0 << 30.0 << 30.0 << 500.0;
        amplitude << 1.0 << 1.0   << 0.0   << 0.0;

        f = FilterLeastSquaresFIR(sr, 32, freqs, amplitude);

        f.setWindow(RECTANGULAR);

        // The dB response is compared with a looser tolerance.
        check_gold(
            f.getFrequencyResponse().getdB(),
            "gold/FilterLeastSquaresFIR_out3.wav",
            1e-9);
    }
}

//! A 1000 Hz sample rate low pass, 1 below 100 Hz and 0 above.
static
FilterLeastSquaresFIR
make_low_pass(uint32 kernel_size)
{
    Buffer freqs;
    Buffer amplitude;
    freqs     << 0.0 << 100.0 << 100.0 << 500.0;
    amplitude << 1.0 << 1.0   << 0.0   << 0.0;

    return FilterLeastSquaresFIR(1000.0, kernel_size, freqs, amplitude);
}

TEST_CASE("FilterLeastSquaresFIR design")
{
    FilterLeastSquaresFIR f = make_low_pass(101);

    SECTION("the axes it was designed with, frequencies over Nyquist")
    {
        check_close(f.getKernelFrequencies(), Buffer({0.0, 0.2, 0.2, 1.0}), 0.0);
        check_close(f.getKernelAmplitudes(), Buffer({1.0, 1.0, 0.0, 0.0}), 0.0);
    }

    SECTION("a symmetric, linear phase kernel")
    {
        Buffer k = f.getKernel();

        REQUIRE(k.getLength() == 101);
        check_close(k, k.getReverse(), 1e-15);

        // The impulse response is the kernel.
        check_close(f.getImpulseResponse().subbuffer(0, 101), k, 0.0);
    }

    SECTION("it passes below 100 Hz and cuts above")
    {
        Buffer h = f.getFrequencyResponse(1024);

        // 1000 / 1024 Hz bins.
        CHECK(std::fabs(h[20] - 1.0) < 0.01);
        CHECK(h[200] < 0.01);
    }
}

TEST_CASE("FilterLeastSquaresFIR makeKernel() checks the axes")
{
    FilterLeastSquaresFIR f = make_low_pass(101);

    Buffer kernel = f.getKernel();
    Buffer freqs = f.getKernelFrequencies();
    Buffer amplitudes = f.getKernelAmplitudes();

    auto check_throws = [&](const Buffer & fa, const Buffer & aa)
    {
        CHECK_THROWS_AS(f.makeKernel(fa, aa), Exception);

        // A failed design leaves the filter as it was.
        check_close(f.getKernel(), kernel, 0.0);
        check_close(f.getKernelFrequencies(), freqs, 0.0);
        check_close(f.getKernelAmplitudes(), amplitudes, 0.0);
    };

    SECTION("different lengths")
    {
        check_throws(Buffer({0.0, 500.0}), Buffer({1.0, 1.0, 0.0, 0.0}));
    }

    SECTION("an odd length")
    {
        check_throws(Buffer({0.0, 250.0, 500.0}), Buffer({1.0, 1.0, 0.0}));
    }

    SECTION("not starting at 0 Hz")
    {
        check_throws(Buffer({10.0, 500.0}), Buffer({1.0, 1.0}));
    }

    SECTION("not ending at Nyquist")
    {
        check_throws(Buffer({0.0, 400.0}), Buffer({1.0, 1.0}));
    }

    SECTION("decreasing frequencies")
    {
        check_throws(Buffer({0.0, 300.0, 200.0, 500.0}), Buffer({1.0, 1.0, 0.0, 0.0}));
    }

    SECTION("bands with gaps between them aren't supported")
    {
        check_throws(Buffer({0.0, 100.0, 150.0, 500.0}), Buffer({1.0, 1.0, 0.0, 0.0}));
    }
}

TEST_CASE("FilterLeastSquaresFIR setKernel()")
{
    FilterLeastSquaresFIR f = make_low_pass(101);

    Buffer impulse = Buffer::zeros(16);
    impulse[0] = 1.0;

    SECTION("a kernel of another size")
    {
        Buffer k({0.5, 0.25, 0.125});

        f.setKernel(k);

        CHECK(f.getKernelSize() == 3);
        check_close(f.getKernel(), k, 0.0);
        check_close(f.filter(impulse).subbuffer(0, 4), Buffer({0.5, 0.25, 0.125, 0.0}), 0.0);
    }

    SECTION("a kernel of the same size")
    {
        Buffer k = 0.01 * Buffer::ones(101);

        f.setKernel(k);

        check_close(f.getKernel(), k, 0.0);
    }
}

TEST_CASE("FilterLeastSquaresFIR small kernels")
{
    Buffer freqs;
    Buffer amplitude;
    freqs     << 0.0 << 100.0 << 100.0 << 500.0;
    amplitude << 1.0 << 1.0   << 0.0   << 0.0;

    SECTION("every size is symmetric")
    {
        for(uint32 n = 1; n < 8; ++n)
        {
            INFO(n << " taps");

            FilterLeastSquaresFIR f(1000.0, n, freqs, amplitude);

            Buffer k = f.getKernel();

            REQUIRE(k.getLength() == n);
            check_close(k, k.getReverse(), 1e-15);
        }
    }

    SECTION("no taps throws")
    {
        CHECK_THROWS_AS(FilterLeastSquaresFIR(1000.0, 0, freqs, amplitude), Exception);

        FilterLeastSquaresFIR f = make_low_pass(101);

        Buffer kernel = f.getKernel();

        CHECK_THROWS_AS(f.setKernel(Buffer()), Exception);

        check_close(f.getKernel(), kernel, 0.0);
    }
}

TEST_CASE("FilterLeastSquaresFIR overloads agree, the frequency is ignored")
{
    FilterSignals s = make_filter_signals();

    Buffer freqs;
    Buffer amplitude;
    freqs     << 0.0 << 500.0 << 500.0 << 4000.0;
    amplitude << 1.0 << 1.0   << 0.0   << 0.0;

    FilterLeastSquaresFIR f(s.sample_rate, 65, freqs, amplitude);

    check_one_cutoff_overloads(f, s.input.subbuffer(0, 2000), 440.0);
}

TEST_CASE("FilterLeastSquaresFIR copies")
{
    FilterLeastSquaresFIR f = make_low_pass(101);

    Buffer x = Buffer::zeros(200);
    x[0] = 1.0;
    x[50] = -0.5;

    Buffer expected = f.filter(x);

    SECTION("the copy constructor")
    {
        FilterLeastSquaresFIR copy(f);

        check_close(copy.filter(x), expected, 0.0);
        check_close(copy.getKernelFrequencies(), f.getKernelFrequencies(), 0.0);
    }

    SECTION("operator= onto a filter with another kernel size")
    {
        FilterLeastSquaresFIR other = make_low_pass(31);

        other = f;

        CHECK(other.getKernelSize() == 101);
        check_close(other.filter(x), expected, 0.0);

        FilterLeastSquaresFIR & same = other;
        other = same;

        check_close(other.filter(x), expected, 0.0);
    }
}

TEST_CASE("FilterLeastSquaresFIR Filter base")
{
    Buffer freqs;
    Buffer amplitude;
    freqs     << 0.0 << 100.0 << 100.0 << 500.0;
    amplitude << 1.0 << 1.0   << 0.0   << 0.0;

    check_filter_base<FilterLeastSquaresFIR>(1000.0, 101u, freqs, amplitude);
}
