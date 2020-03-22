#!/bin/sh
# Must run this script with sudo

echo "Register DS3231 as i2c device"
echo ds1307 0x68 > /sys/class/i2c-adapter/i2c-1/new_device
echo "Set time using RTC"
hwclock -s
echo "Done"

exit 0
