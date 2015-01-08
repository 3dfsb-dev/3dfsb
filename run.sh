#!/bin/sh
# Quick and dirty script that I use to run 3dfsb,
# because I've compiled gstreamer-1.xx from source and installed it in /usr/local/lib/
LD_LIBRARY_PATH=/usr/local/lib/gstreamer-1.0  ./3dfsb
