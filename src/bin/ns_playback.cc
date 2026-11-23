//-----------------------------------------------------------------------------
//
//  ns_playback: lists portaudio's host APIs, or plays a test sound through
//  one with AudioPlaybackRt, to hear which plays cleanly.
//
//      bazel run //src/bin:ns_playback                          # lists them
//      bazel run //src/bin:ns_playback -- "Windows WASAPI"      # plays
//      bazel run //src/bin:ns_playback -- "Windows WDM-KS" 0.02 48000 2
//
//  The arguments after the host API are the buffer seconds, 0.05 by
//  default, the sample rate and the channels, by default the default output
//  device's sample rate and 2 channels, or 1 when it has 1.  Some host APIs,
//  WDM-KS among them, only play the formats the device does.
//
//  The test sound is a sine sweep from 220 to 880 Hz, a pure tone, where a
//  glitch is heard as a click.  After it plays, the player's settings and
//  underrun counts are printed.
//
//-----------------------------------------------------------------------------

#include <Nsound/NsoundAll.h>

#ifdef NSOUND_LIBPORTAUDIO
    #include <portaudio.h>
#endif

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace Nsound;

#ifdef NSOUND_LIBPORTAUDIO

//! Returns s in lower case.
static
std::string
lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

//! Returns the default output device of the host API named, nullptr when
//! there is none.  portaudio must be initialized.
static
const PaDeviceInfo *
default_output_device(const std::string & host_api)
{
    for(PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); ++i)
    {
        const PaHostApiInfo * api = Pa_GetHostApiInfo(i);

        if(api != nullptr && lower(api->name) == lower(host_api)
            && api->defaultOutputDevice != paNoDevice)
        {
            return Pa_GetDeviceInfo(api->defaultOutputDevice);
        }
    }

    return nullptr;
}

//! Lists the host APIs and their default output devices.
static
void
list_host_apis()
{
    std::cout << "The portaudio host APIs and their default output devices,\n"
              << "play through one with:\n"
              << "    ns_playback \"<host API>\" [buffer seconds] [sample rate] [channels]\n\n";

    Pa_Initialize();

    for(const std::string & name : AudioPlaybackRt::getHostApis())
    {
        std::cout << "    " << name << "\n";

        const PaDeviceInfo * device = default_output_device(name);

        if(device == nullptr)
        {
            std::cout << "        no output device\n";
        }
        else
        {
            std::cout << "        " << device->name << ", "
                      << device->defaultSampleRate << " Hz, "
                      << device->maxOutputChannels << " channels\n";
        }
    }

    Pa_Terminate();
}

int
main(int argc, char ** argv)
{
    if(argc < 2)
    {
        list_host_apis();
        return 0;
    }

    std::string host_api = argv[1];

    float64 buffer_seconds = argc > 2 ? std::atof(argv[2]) : 0.05;

    // The device's format by default.
    float64 sr = 44100.0;
    uint32 channels = 2;

    Pa_Initialize();

    if(const PaDeviceInfo * device = default_output_device(host_api))
    {
        sr = device->defaultSampleRate;
        channels = std::min(2, device->maxOutputChannels);
    }

    Pa_Terminate();

    if(argc > 3) sr = std::atof(argv[3]);
    if(argc > 4) channels = static_cast<uint32>(std::atoi(argv[4]));

    try
    {
        Sine sine(sr);

        // 4 seconds, quiet enough not to startle, faded in and out.
        Buffer sweep = 0.3 * sine.generate(4.0, sine.drawLine(4.0, 220.0, 880.0));

        sweep *= sine.drawLine(0.05, 0.0, 1.0)
              << sine.drawLine(3.9, 1.0, 1.0)
              << sine.drawLine(0.05, 1.0, 0.0);

        AudioPlaybackRt::use_host_api(host_api);

        AudioPlaybackRt pb(sr, channels, 3, buffer_seconds);

        std::cout << "Playing a sweep through '" << host_api << "', "
                  << sr << " Hz, " << channels << " channels ...\n";

        // A mono sample plays on every channel.
        pb.play(sweep);

        std::cout << pb.debug_print() << std::endl;
    }
    catch(const Exception &)
    {
        // Nsound printed it.
        return 1;
    }
    catch(const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#else

int
main()
{
    std::cerr << "Nsound was built without portaudio, --//bazel:portaudio=false\n";
    return 1;
}

#endif
