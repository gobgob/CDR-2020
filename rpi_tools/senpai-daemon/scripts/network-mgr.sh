#!/bin/sh

# Set to true to start with the hotspot mode, false to start in standard mode
# (connect to an existing network).
# In any cases, if the standard mode fails it will fallback to hotspot mode.
START_AS_HOTSPOT=false

# Diode std-mode
gpio mode 2 out
gpio write 2 0

# Button std-mode
gpio mode 0 in
gpio mode 0 up

# Diode hotspot-mode
gpio mode 4 out
gpio write 4 0

# Button hotspot-mode
gpio mode 3 in
gpio mode 3 up

sudo systemctl daemon-reload

home="/home/pi/senpai-daemon/"
scripts="${home}scripts/"
ll_conf="${home}conf/low-level-server.conf"

is_std=false
last_ip=""

if [ "$START_AS_HOTSPOT" = true ]; then
    ${scripts}net-hotspot-mode.sh
else
    ${scripts}net-std-mode.sh
    if [ $? -eq 0 ]; then
        is_std=true
    else
        ${scripts}net-hotspot-mode.sh
    fi
fi

while true; do
    if [ "$is_std" = true ]; then
        if [ $(gpio read 3) -eq 0 ]; then
            # button hotspot pressed
            ${scripts}net-hotspot-mode.sh
            if [ $? -eq 0 ]; then
                is_std=false
            fi
        fi
    else
        if [ $(gpio read 0) -eq 0 ]; then
            # button std pressed
            ${scripts}net-std-mode.sh
            if [ $? -eq 0 ]; then
                is_std=true
            else
                ${scripts}net-hotspot-mode.sh
            fi
        fi
    fi

    ip=`ip -4 address show dev wlan0 up | grep -o "inet [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*" | grep -o "[.0-9]*" | head -n 1`
    if [ "$ip" != "$last_ip" ]; then
        last_ip=${ip}
        echo "New IP address:" "$ip"

        # Update LowLevelServer configuration
        ${scripts}make-config-file.py ${ll_conf} --ip ${ip}
        ${scripts}low-level-server-restart.py
    fi

    sleep 0.2
done

exit 0
