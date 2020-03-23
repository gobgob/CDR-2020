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

# sudo systemctl daemon-reload # useful ?

scripts="/home/pi/senpai-daemon/scripts/"

is_std=false
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
    sleep 0.1
done

exit 0
