#!/bin/sh

bin="/home/pi/senpai-daemon/bin/"
scripts="/home/pi/senpai-daemon/scripts/"
log="/home/pi/senpai-daemon/log/"

${scripts}network-mgr.sh > ${log}network-mgr.log 2>&1 &
sudo ${scripts}rtc-init.sh > ${log}rtc.log 2>&1 &
${scripts}gpio_halt.sh > ${log}gpio_halt.log 2>&1 &
${scripts}low-level-server.py ${bin}LowLevelServer ${log}low-level-server.log &

exit 0
