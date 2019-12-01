#!/bin/sh
java -Xmx1G -Xms1G -cp target/navire-hl.jar senpai.CapteursTest $@ | tee last_all.txt
