###############################################################################
#
#  $Id: example5.py 811 2013-12-27 00:15:15Z weegreenblobbie $
#
###############################################################################

from Nsound import *

voice = AudioStream("california.wav")

sr = voice.getSampleRate()

# Get rid of some low frequency noise from the recording.
hp = FilterHighPassIIR(sr, 6, 45.0, 0.01)

voice = hp.filter(voice)

voice_dur = voice.getDuration()

gen = Granulator(sr, Granulator.DECAY)

carrier = gen.generate(voice_dur, 1500.0, 6, 40.0)

vocod = Vocoder(sr, 0.020, 16, 8000)

output = vocod.filter(voice[0], carrier)

output.normalize()

output *= 0.666

# Play to sound card
pb = AudioPlayback(sr, 2, 16)

output >> "example5.wav"
output >> pb

###############################################################################
# Create some spectrogram plots.

window = 0.080 # seconds
step   = 0.020 # seconds

pylab = Plotter()

pylab.figure()
ax1 = pylab.subplot(2, 1, 1)

spec1 = Spectrogram(voice[0], sr, window, step, HANNING);

pylab.imagesc(
    spec1.getTimeAxis(),
    spec1.getFrequencyAxis(),
    spec1.getMagnitude().getTranspose())

fmax = 2000.0

pylab.ylim(0.0, fmax)
pylab.xlim(0.0, voice_dur)

pylab.title("Voice input")

pylab.subplot(2, 1, 2, "", ax1, ax1)

spec2 = Spectrogram (output, sr, window, step, HANNING)

pylab.imagesc(
    spec2.getTimeAxis(),
    spec2.getFrequencyAxis(),
    spec2.getMagnitude().getTranspose())

pylab.ylim(0.0, fmax)
pylab.xlim(0.0, voice_dur)

pylab.title("Voice output")

pylab.show()
