//-----------------------------------------------------------------------------
//
//  $Id: example5.cc 874 2014-09-08 02:21:29Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Nsound headers
#include <Nsound/NsoundAll.h>

using namespace Nsound;

using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
int
my_main(void)
{
    AudioStream voice("california.wav");

    float64 sr = voice.getSampleRate();

    Wavefile::setDefaultSampleRate(sr);

    // Get rid of some annoying frequencies from the recording.
    FilterBandPassIIR bpf(sr, 6, 45.0, 16000.0, 0.01);

    voice = bpf.filter(voice);

    float64 voice_dur = voice.getDuration();

    // The spectral rich granulator is used carrier frequency.
    Granulator gen(sr, Granulator::REVERSE_DECAY);

    Buffer carrier = gen.generate(voice_dur, 16000.0, 3, 40.0);

    // The vocoder processes the input with a filter bank.  The end result
    // should have a similar spectral distrubution of the energy, but with a
    // timbre.
    //
    Vocoder vocod(sr, 0.0050, 32, 4000);

    Buffer output = vocod.filter(voice[0], carrier);

    output.normalize();

    output *= 0.666;

    output >> "example5.wav";

    try
    {
        AudioPlayback pb(sr, 2, 16);
        output >> pb;
    }
    catch(Exception e)
    {
        cerr << "Could not play audio: " << e.what() << endl;
    }

    // Create some spectrogram plots.

    Plotter pylab;

    pylab.figure();
    Axes ax1 = pylab.subplot(2, 1, 1);

    float64 window = 0.080; // seconds
    float64 step   = 0.020; // seconds

    Spectrogram spec1(voice[0], sr, window, step, HANNING);

    pylab.imagesc(
        spec1.getTimeAxis(),
        spec1.getFrequencyAxis(),
        spec1.getMagnitude().getTranspose());

    float64 fmax = 2000.0;

    pylab.ylim(0.0, fmax);
    pylab.xlim(0.0, voice_dur);

    pylab.title("Voice input");

    pylab.subplot(2, 1, 2, /*kwargs=*/ "", /*sharex=*/ &ax1, /*sharey=*/ &ax1);

    Spectrogram spec2(output, sr, window, step, HANNING);

    pylab.imagesc(
        spec2.getTimeAxis(),
        spec2.getFrequencyAxis(),
        spec2.getMagnitude().getTranspose());

    pylab.ylim(0.0, fmax);
    pylab.xlim(0.0, voice_dur);

    pylab.title("Voice output");

    pylab.show();

    return 0;
}

int main(int /*argc*/, char ** /*argv*/)
{
	try
	{
		my_main();
		return 0;
	}
	catch(std::exception & e)
	{
		cerr << "Exception: " << e.what() << endl;
		return 1;
	}

	return 0;
}

