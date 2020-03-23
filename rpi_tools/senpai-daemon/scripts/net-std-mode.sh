#!/bin/sh

dhcpconf="/home/pi/senpai-daemon/conf/dhcp-conf-std/dhcpcd.conf"

# Blink to indicate the start
gpio write 2 1 # LED std ON
sleep 0.2
gpio write 2 0 # LED std OFF

# Stop hotspot
echo "Stop hotspot"
sudo systemctl stop hostapd
gpio write 4 0 # LED hotspot OFF

# Set standard DHCP conf
echo "Set standard DHCP conf"
sudo cp ${dhcpconf} /etc/
echo "Restart dhcpcd service"
sudo service dhcpcd restart
echo "Reconfigure wlan0"
wpa_cli -i wlan0 reconfigure > /dev/null

# Check connection status for 10 seconds
for i in `seq 1 10`; do
    gpio write 2 1 # LED std ON

    for j in `seq 1 5`; do
        if [ $(gpio read 3) -eq 0 ]; then
            # button hotspot pressed
            gpio write 2 0 # LED std OFF
            exit 0
        fi
        sleep 0.1
    done

    gpio write 2 0 # LED std OFF

    for j in `seq 1 5`; do
        if [ $(gpio read 3) -eq 0 ]; then
            # button hotspot pressed
            exit 0
        fi
        sleep 0.1
    done

    status=$(cat /sys/class/net/wlan0/operstate)
    if [ "$status" = "up" ]; then
        echo "wlan0: connection established"
        gpio write 2 1 # LED std ON
        exit 0
    fi
    echo "wlan0: waiting for connection"
done
echo "wlan0: failed to connect"
exit 1
