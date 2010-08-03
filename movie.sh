#!/bin/sh
rm -f foamMovie.mp4
ffmpeg -r 1 -b 200k -i movie/frame%04d.jpg foamMovie.mp4
smplayer foamMovie.mp4
