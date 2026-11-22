//-----------------------------------------------------------------------------
//
//  GeneratorDecay tests, exp(-alpha t) restarting every wavelength, t the
//  fraction of the wavelength.
//
//-----------------------------------------------------------------------------

#include <Nsound/Buffer.h>
#include <Nsound/GeneratorDecay.h>
#include <Nsound/Sine.h>

#include "gold_check.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Nsound;

//! exp(-alpha t) with t the fraction of the wavelength, stepping f / sr per
//! sample.
static
Buffer
decays(uint32 n, float64 sr, float64 f, float64 alpha)
{
    Buffer b;
    for(uint32 i = 0; i < n; ++i)
    {
        b << std::exp(-alpha * std::fmod(f * i, sr) / sr);
    }
    return b;
}

TEST_CASE("GeneratorDecay restarts every wavelength")
{
    GeneratorDecay d(100.0);

    SECTION("a whole number of samples per wavelength")
    {
        // It restarted at 51, 101 and 151.
        check_close(d.generate2(2.0, 2.0, 1.0), decays(200, 100.0, 2.0, 1.0), 1e-15);
    }

    SECTION("wavelengths between samples keep their phase")
    {
        check_close(d.generate2(1.0, 3.0, 2.0), decays(100, 100.0, 3.0, 2.0), 1e-15);
        check_close(d.generate2(1.0, 2.5, 0.5), decays(100, 100.0, 2.5, 0.5), 1e-15);
    }

    SECTION("generate() decays with alpha 1")
    {
        check_close(d.generate(1.0, 2.0), decays(100, 100.0, 2.0, 1.0), 1e-15);
    }
}

TEST_CASE("GeneratorDecay overloads agree")
{
    GeneratorDecay d(100.0);

    Buffer expected = d.generate2(1.0, 2.0, 3.0);

    Buffer freqs = 2.0 * Buffer::ones(100);
    Buffer alphas = 3.0 * Buffer::ones(100);

    check_close(d.generate2(1.0, 2.0, alphas), expected, 0.0);
    check_close(d.generate2(1.0, freqs, 3.0), expected, 0.0);
    check_close(d.generate2(1.0, freqs, alphas), expected, 0.0);

    d.reset();

    Buffer per_sample;
    for(uint32 i = 0; i < 100; ++i) per_sample << d.generate2(2.0, 3.0);

    check_close(per_sample, expected, 0.0);
}

TEST_CASE("GeneratorDecay synced to a master restarts with it")
{
    // A 3 Hz master restarts a 1 Hz decay where its wavelength ends, after
    // 34 and 67 samples.
    Sine master(100.0);
    GeneratorDecay slave(100.0);

    master.setRealtime(true);
    slave.setRealtime(true);
    master.addSlaveSync(slave);

    Buffer y;
    for(uint32 i = 0; i < 100; ++i)
    {
        master.generate(3.0);
        y << slave.generate2(1.0, 1.0);
    }

    // It never restarted.
    CHECK(y[33] == 1.0);
    CHECK(y[66] == 1.0);
    check_close(y.subbuffer(0, 33), decays(33, 100.0, 1.0, 1.0), 1e-15);
    check_close(y.subbuffer(33, 33), decays(33, 100.0, 1.0, 1.0), 1e-15);
}

TEST_CASE("GeneratorDecay as a master restarts its slaves")
{
    // A 3 Hz decay restarts a 1 Hz sine on the sample its wavelength ends, the
    // 34th.
    GeneratorDecay master(100.0);
    Sine slave(100.0);

    master.setRealtime(true);
    slave.setRealtime(true);
    master.addSlaveSync(slave);

    Buffer y;
    for(uint32 i = 0; i < 40; ++i)
    {
        master.generate2(3.0, 1.0);
        y << slave.generate(1.0);
    }

    Sine sine(100.0);
    Buffer expected = sine.generate(1.0, 1.0);

    check_close(y.subbuffer(0, 33), expected.subbuffer(0, 33), 1e-15);
    check_close(y.subbuffer(33), expected.subbuffer(0, 7), 1e-15);
}
