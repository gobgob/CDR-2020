#!/bin/sh

conf="/home/pi/senpai-daemon/conf/"
sudo systemctl daemon-reload

# Stop hotspot
echo "Stop hotspot"
sudo systemctl stop hostapd
# Set standard DHCP conf
echo "Set standard DHCP conf"
sudo cp ${conf}dhcp-conf-std/dhcpcd.conf /etc/
echo "Restart dhcpcd service"
sudo service dhcpcd restart
echo "Reconfigure wlan0"
wpa_cli -i wlan0 reconfigure > /dev/null

# Check connection status for 10 seconds
for i in `seq 1 10`; do
    sleep 1
    status=$(cat /sys/class/net/wlan0/operstate)
    if [ "$status" = "up" ]; then
        echo "wlan0: connection established"
        exit 0
    fi
    echo "wlan0: waiting for connection"
done
echo "wlan0: failed to connect"

# Set static-ip DHCP conf
echo "Set static-IP DHCP conf"
sudo cp ${conf}dhcp-conf-static/dhcpcd.conf /etc/
echo "Restart dhcpcd service"
sudo service dhcpcd restart

# Start hotspot
echo "Start hostapd service (hotspot)"
sudo systemctl start hostapd
echo "Done"

exit 0
