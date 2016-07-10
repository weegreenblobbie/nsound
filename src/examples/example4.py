###############################################################################
#
#  $Id: example4.py 718 2012-04-15 23:59:35Z weegreenblobbie $
#
###############################################################################

import Nsound as ns

sr = 44100.0

sine = ns.Sine(sr)

bass = ns.GuitarBass(sr)
slide = ns.FluteSlide(sr)
clarinet = ns.Clarinet(sr)

out = ns.AudioStream(sr, 1)

out << bass.play()      << sine.silence(1.0) \
    << slide.play()     << sine.silence(1.0) \
    << clarinet.play()  << sine.silence(1.0)

out *= 0.5

# Set the default Wavefile sample size and rate.
ns.Wavefile.setDefaults(sr, 16)

out >> "example4.wav"

room = ns.ReverberationRoom(sr, 0.9); # 0.9 = room feed back (0.0 to 1.0)

out2 = ns.AudioStream(sr, 1)
out2 << bass.play() << sine.silence(1.5)

out2 = room.filter(0.5 * out2)

out2 >> "example4_reverb.wav"

# Play to audio device.

pb = ns.AudioPlayback(sr, 2, 16)

out >> pb
out2 >> pb
