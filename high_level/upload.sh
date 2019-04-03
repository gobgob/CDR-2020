#!/bin/sh
mvn clean compile assembly:single
scp target/chariot-hl.jar pi@172.24.1.1:eurobotruck/high_level/target
scp target/chariot-hl.jar pi@172.24.1.1:
