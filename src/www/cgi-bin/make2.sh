#!/bin/sh
echo "Compile nexus.c into a deamon to be used by apache"
echo " + gcc.exe -o nexus.exe nexus.c -lwsock32"
gcc.exe -o nexus.exe nexus.c -lwsock32
echo " + gcc.exe -o sender.exe sender.c -lwsock32"
gcc.exe -o sender.exe sender.c -lwsock32
