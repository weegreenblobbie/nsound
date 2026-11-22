//-----------------------------------------------------------------------------
//
//  Compares a test's output with a gold file.
//
//-----------------------------------------------------------------------------
#pragma once

#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>

#include <catch2/catch_test_macros.hpp>

#include <string>

//! Checks that data has expected's length and that no sample differs from it
//! by more than tolerance.
inline
void
check_close(
    const Nsound::Buffer & data,
    const Nsound::Buffer & expected,
    Nsound::float64 tolerance)
{
    REQUIRE(data.getLength() == expected.getLength());

    // Two empty buffers match, and an empty buffer has no max.
    if(expected.getLength() > 0)
    {
        CHECK((data - expected).getAbs().getMax() <= tolerance);
    }
}

//! check_close() against a gold file.
inline
void
check_gold(
    const Nsound::Buffer & data,
    const std::string & gold_file,
    Nsound::float64 tolerance)
{
    INFO("gold file: " << gold_file);

    check_close(data, Nsound::Buffer(gold_file), tolerance);
}

//! Checks that data has the gold file's channels and length and that no
//! sample differs from it by more than tolerance.
inline
void
check_gold(
    const Nsound::AudioStream & data,
    const std::string & gold_file,
    Nsound::float64 tolerance)
{
    Nsound::AudioStream gold(gold_file);

    INFO("gold file: " << gold_file);

    REQUIRE(data.getNChannels() == gold.getNChannels());
    REQUIRE(data.getLength() == gold.getLength());
    CHECK((data - gold).getAbs().getMax() <= tolerance);
}

