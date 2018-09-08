#!/bin/sh
echo "Compile arduino.c into a CGI to be used by apache"
echo " + gcc.exe -o arduino.cgi arduino.c"
gcc.exe -o arduino.cgi arduino.c -lwsock32
