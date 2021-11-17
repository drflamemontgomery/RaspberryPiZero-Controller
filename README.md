# Raspberry Pi Zero Controller

<h2>First Time Setup</h2>

Put hid.sh and tcp.c in /home/pi<br>
Add lines from rc.local to /etc/rc.local<br>
<br>
chmod +x setup.sh<br>
sudo ./setup.sh<br>
chmod +x  hid.sh<br>
gcc -o tcp tcp.c -lpthread<br>

[Heaps.io TCP RPI Controller](https://github.com/drflamemontgomery/rpicontroller)
