#!/bin/sh
java -Xmx1G -Xms1G -cp target/chariot-hl.jar senpai.CapteursTest $@ | tee last_all.txt
