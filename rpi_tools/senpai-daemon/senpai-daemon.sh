#!/bin/sh

home="/home/pi/senpai-daemon/"
bin="${home}bin/"
scripts="${home}scripts/"
log="${home}log/"
ll_output="/home/pi/senpai-logs/"

rm ${log}old/*
mv ${log}* ${log}old/

unbuffer ${scripts}network-mgr.sh > ${log}network-mgr.log 2>&1 &
unbuffer sudo ${scripts}rtc-init.sh > ${log}rtc.log 2>&1 &
unbuffer ${scripts}gpio_halt.sh > ${log}gpio_halt.log 2>&1 &
unbuffer sudo ${bin}LowLevelServer -l ${ll_output} > ${log}low-level-server.log 2>&1 &

exit 0
