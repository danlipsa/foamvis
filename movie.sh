#!/bin/sh
rm -f foamMovie.mp4
ffmpeg -r 5 -b 3000k -i movie/frame%04d.png foamMovie.mp4
smplayer foamMovie.mp4
