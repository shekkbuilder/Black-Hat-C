#!/bin/bash
echo "[*] Installing mingw-w64"
apt-get install mingw-w64 -y
echo "[*] Installing multilib"
apt-get install gcc-multilib -y
echo "[*] Dependencies installed"
