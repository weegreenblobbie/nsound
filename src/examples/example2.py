###############################################################################
#
#  $Id: example2.py 718 2012-04-15 23:59:35Z weegreenblobbie $
#
###############################################################################

import Nsound as ns

sr = 44100.0

# Create a new instance of the Sine Generator
sine = ns.Sine(sr)

pan = ns.Buffer()

pan << sine.generate(4.9, 3.0)

# Create a stereo AudioStream.
a = ns.AudioStream(sr, 2)

# Fill it with a 220 Hz sine wave.
a << 0.5 * sine.generate(4.9, 220)

# Execute the pan method.
a.pan(pan)

# Write the AudioStream to a wave file
a >> "example2.wav";

a >> ns.AudioPlayback(sr, 2, 16);

