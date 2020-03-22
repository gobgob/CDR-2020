#!/bin/sh

gpio mode 22 out # diode en output
gpio mode 21 in # interrupteur en input
gpio mode 21 up # interrupteur pull-up
gpio write 22 0 # diode éteinte par défaut

stop=0
while [ $stop -eq 0 ]; do
    # On attend un appui sur le bouton
    gpio wfi 21 falling
    stop=1
    # Le bouton doit rester appuyé pendant 1 seconde
    for i in `seq 1 10`; do
        sleep 0.1
        if [ $(gpio read 21) -eq 1 ]; then
            # bouton relaché
            stop=0
            break
        fi
    done
done

# on prévient les clients ssh de l'arrêt imminent
wall "Arrêt du système par GPIO !"

# blink pour montrer qu'on a bien reçu la demande d'arrêt
gpio write 22 1
sleep 0.3
gpio write 22 0
sleep 0.3

# on éteint le HL et on laisse un peu de temps au HL pour s'éteindre (s'il était déjà éteint, pas de sleep)
pkill -f navire-hl.jar && sleep 3

# arrêt
sudo halt

# la diode sera allumée une fois la raspi complètement éteinte (ou pas ^^)
