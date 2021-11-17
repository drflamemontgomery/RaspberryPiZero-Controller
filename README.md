# Raspberry Pi Zero Controller

<h2>First Time Setup</h2>

Put hid.sh and tcp.c in /home/pi
Add lines from rc.local to /etc/rc.local

chmod +x setup.sh
sudo ./setup.sh
chmod +x  hid.sh
gcc -o tcp tcp.c -lpthread

[Heaps.io TCP RPI Controller](https://github.com/drflamemontgomery/rpicontroller)