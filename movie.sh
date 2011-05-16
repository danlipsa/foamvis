#!/bin/sh
rm -f foamMovie.mp4
ffmpeg -r 10 -mbd rd -trellis 2 -cmp 2 -subcmp 2 -g 300 -pass 1/2 -i movie/frame%04d.png foamMovie.mp4
smplayer foamMovie.mp4
