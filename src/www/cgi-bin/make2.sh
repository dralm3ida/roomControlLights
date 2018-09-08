#!/bin/sh
echo "Compile nexus.c into a deamon to be used by apache"
echo " + gcc.exe -o nexus.exe nexus.c -lwsock32"
gcc.exe -o nexus.exe nexus.c -lwsock32
