###############################################################################
#
#  $Id: example1.py 718 2012-04-15 23:59:35Z weegreenblobbie $
#
###############################################################################

# Import the Nsound module
import Nsound as ns

sr = 44100.0

# Creating the Pac Man background tone.

sine = ns.Sine(sr)

time = 0.40
h_time = time / 2.0

# The first tone.
f1 = ns.Buffer()
f1 << sine.drawLine(h_time, 345, 923) \
   << sine.drawLine(h_time, 923, 345)

pac_man = ns.AudioStream(sr, 1)
pac_man << sine.generate(7.0 * time, f1)

# Here we create an envelop to smoothly finish the waveform, removing
# any clicking that might have occured.
envelope = ns.AudioStream(sr, 1)
envelope << sine.drawLine(7*time-0.005, 1.0, 1.0) \
         << sine.drawLine(0.005, 1.0, 0.0)

pac_man *= envelope

pac_man.normalize()
pac_man *= 0.25

pac_man >> "example1.wav"

# Play to audio device.

pac_man >> ns.AudioPlayback(sr, 1, 16)

