import datetime
import os
import subprocess

import Nsound as ns

def save_ogg(wav, name, remove_wav=True):
    wavfile = f"source/_static/{name}.wav"
    oggfile = wavfile.replace(".wav", ".ogg")
    if not os.path.isfile(oggfile):
        wav >> wavfile
        subprocess.check_call(
            ["oggenc", "-q", "10", wavfile,
                "--title", name,
                "--artist", "Nsound",
                "--album", ns.__version__,
                "--date", datetime.datetime.now().strftime("%Y-%m-%d"),
                "--comment", "https://github.com/weegreenblobbie/nsound",
            ]
        )
        if remove_wav:
            os.remove(wavfile)

    return f"Sample here: `{name}.ogg <_static/{name}.ogg>`_"
