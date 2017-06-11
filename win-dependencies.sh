#!/bin/bash
#
# Script name	: win-dependencies.sh
# Ubuntu Ver	: Ubuntu 17.04 (Works On Pretty Much Any Linux distrib.)
# Author	: wetw0rk
# How2Run	: chmod +x win-dependencies.sh && sudo ./win-dependencies.sh
# Last Updated	: 7/11/2017
# Descrption	: Simply installs dependencies for the example programs within
#		  Black-Hat-C. This mainly is needed for the Windows versions
#		  of the program examples. Enjoy :)
#
#


echo "[*] Installing mingw-w64"
apt-get install mingw-w64 -y
echo "[*] Installing multilib"
apt-get install gcc-multilib -y
echo "[*] Dependencies installed"
