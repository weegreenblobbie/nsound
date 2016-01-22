//-----------------------------------------------------------------------------
//
//  $Id: cepstral_pitch_estimate.cc 878 2014-11-23 04:51:23Z weegreenblobbie $
//
//-----------------------------------------------------------------------------

// Include the Nsound headers
#include <Nsound/NsoundAll.h>

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace Nsound;

int
my_main(void)
{
    Plotter pylab;

    // Region of the spectrogram to pull out for cepstral analysis.
    float64 FMIN = 80.0;
    float64 FMAX = 2000.0;

    // The maximum pitch value to consider when searching for the peak in the
    // cepstrum analysis.
    float64 PMAX = 600.0;

    // Read in the target wavefile.

    std::string input_filename = "walle.wav";

    AudioStream a(input_filename);

    // Since the sample is speech, I can downsample it to 8 kHz to speed up
    // the calculations.  Most of the harmonic information is less than 4 kHz.

    float64 sr = a.getSampleRate();

    float64 ratio = 8000.0 / sr;

    a.resample(ratio);

    sr = 8000.0;
    a.setSampleRate(static_cast<uint32>(sr));

    // Calculate the STFT a.k.a. the spectrogram.

    Spectrogram spec(a[0], sr, 0.060, 0.010, HANNING);

    Buffer freq_axis = spec.getFrequencyAxis();
//~    float64 df = freq_axis[1] - freq_axis[0];

    Buffer time_axis = spec.getTimeAxis();
    uint32 n_time_steps = time_axis.getLength();

    AudioStream magnitude = spec.getMagnitude();

    // N = FFT size.
    uint32 N = magnitude.getLength() * 2;

    // Cepstrum size.
    uint32 M = 2 * N;

    cerr << "N = " << N << endl;
    cerr << "M = " << M << endl;
    cerr << "M/2 + 1 = " << (M / 2 + 1) << endl;

    cerr << "spectrum.size = " << magnitude.getNChannels() << endl;

    // Determine the indicies of the spectrogram to pull out for cepstral
    // analysis.

    float64 vmin = freq_axis[0];
    float64 vstep = freq_axis[1] - vmin;

    uint32 fmin_index = static_cast<uint32>( (FMIN - vmin) / vstep + 0.5);
    uint32 fmax_index = static_cast<uint32>( (FMAX - vmin) / vstep + 0.5);

    Generator gen(1);

    Buffer n = gen.drawLine(M / 2 + 1, 0.0, M / 2 + 1);

    n[0] = 0.1;

    Buffer quefrency_axis = (N * n) / (M * sr);

    Buffer pitch_axis = 1.0 / quefrency_axis;

    cerr << "quefrency_axis.getLength() = " << quefrency_axis.getLength() << endl;
    cerr << "pitch_axis.getLength() = " << pitch_axis.getLength() << endl;

    // Find the index for PMAX

    uint32 pmax_index = (pitch_axis - PMAX).getAbs().argmin();

    // Get pitch_axis range of interest.
    pitch_axis = pitch_axis.subbuffer(pmax_index);

    // Extract the pitch for each time step.

    FFTransform fft(sr); // really the sample rate is never used for anything.

    // Create a Hanning window
    Buffer window = gen.drawWindowHanning(M / 2 + 1);

    Buffer pitch_path;

    boolean once = true;

    AudioStream cep_space(1.0, n_time_steps);

    for(uint32 i = 0; i < n_time_steps; ++i)
    {
        Buffer spectrum = magnitude[i];

        // Pull out region of the spectrum from FMIN to FMAX
        spectrum = spectrum.subbuffer(fmin_index, fmax_index - fmin_index);

        // Convert to dB.
        spectrum = 10.0 * (1.0 + spectrum).getdB();

//~        // Subtract mean and window
//~        spectrum -= spectrum.getMean();
//~        spectrum *= window;

        // Cepstral
        FFTChunkVector vec = fft.fft(spectrum, M);

        Buffer cepstrum = vec[0].getMagnitude();

        if(once)
        {
            once = false;
            cerr << "cepstrum.getLength() = " << cepstrum.getLength() << endl;
        }

        // Pull out region of interest.
        cepstrum = cepstrum.subbuffer(pmax_index);

        uint32 p_index = cepstrum.argmax();

        pitch_path << pitch_axis[p_index];

        cep_space[i] << cepstrum;
    }

    // Plot spectrogram + 20 harmonics

    spec.plot(input_filename);

    for(float64 i = 1.0; i <= 20.0; i += 1.0)
    {
        pylab.plot(time_axis, i * pitch_path, "r+-");
    }

    // Set some limits
    pylab.ylim(0, 2000.0);
    pylab.xlim(0.4, 1.6);

    pylab.title("Spectrogram + Estimated Pitch Path");

    // Plot the cepstral matrix, where the peak was pulled out for the pitch
    // estimate.

    pylab.figure();

    Buffer bin_axis = gen.drawLine(
        cep_space.getLength(), 0, cep_space.getLength());

    cep_space.transpose();

    cout << "cep_space.getLength() = " << cep_space.getLength() << endl;
    cout << "cep_space.getNChannels() = " << cep_space.getNChannels() << endl;
    cout << "time_axis.getLength() = " << time_axis.getLength() << endl;
    cout << "bin_axis.getLength() = " << bin_axis.getLength() << endl;

    pylab.imagesc(time_axis, bin_axis, cep_space);
    pylab.title("Cepstrum");
    pylab.xlabel("Time (s)");
    pylab.ylabel("Cepstral bin (not frequency!)");
    // Set some limits
    pylab.xlim(0.4, 1.6);

    Plotter::show();

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
