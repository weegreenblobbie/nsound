#! /usr/bin/env bash

###############################################################################
#
# $Id: getWavefiles.bash 595 2010-12-31 16:24:32Z weegreenblobbie $
#
# Tries to get the example wavefiles from the Nsound Subversion repository.
#
###############################################################################

SVN_PREFIX="https://nsound.svn.sourceforge.net/svnroot/nsound/tags/nsound-0.7.4/src/examples"

echo "Attempting to get wave files from Nsound SVN ..."

FILES=(\
    "mynameis.wav"
    "california.wav"
    "walle.wav"
    "Temperature_in.wav")

for f in ${FILES[@]}; do

    echo "wget $SVN_PREFIX/$f"

    wget "$SVN_PREFIX/$f"

done

ls -l *.wav

echo "All done!"
