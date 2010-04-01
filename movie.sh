#!/bin/sh
rm -f foamMovie.mp4
ffmpeg -r 10 -b 200k -i movie/frame%04d.jpg foamMovie.mp4
mplayer foamMovie.mp4
