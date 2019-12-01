#!/bin/sh
java -Xmx1G -Xms1G -cp target/navire-hl.jar senpai.Match $@ | tee last_all.txt
java -Xmx1G -Xms1G -cp target/navire-hl.jar senpai.Smoke 20000
