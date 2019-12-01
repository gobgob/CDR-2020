#!/bin/sh
mvn clean compile assembly:single
scp target/navire-hl.jar pi@172.24.1.1:eurobotruck/high_level/target
