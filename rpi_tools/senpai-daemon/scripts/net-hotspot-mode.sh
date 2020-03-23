#!/bin/sh

dhcpconf="/home/pi/senpai-daemon/conf/dhcp-conf-static/dhcpcd.conf"

# Blink to indicate the start
gpio write 4 1 # LED hotspot ON
sleep 0.2
gpio write 4 0 # LED hotspot OFF

gpio write 2 0 # LED std OFF

# Set static-ip DHCP conf
echo "Set static-IP DHCP conf"
sudo cp ${dhcpconf} /etc/
echo "Restart dhcpcd service"
sudo service dhcpcd restart

# Start hotspot
echo "Start hostapd service (hotspot)"
sudo systemctl start hostapd
echo "Done"

gpio write 4 1 # LED hotspot ON

exit 0
