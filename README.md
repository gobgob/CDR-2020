# Robot anomyme 2020
Code du robot de GobGob Senpaï pour Eurobot 2020

## Code bas niveau (`low_level/`)

Programme en charge des tâches dites "bas niveau", directement liées au matériel et effectuées en temps réel.  
Notamment :
* L'odométrie
* L'asservissement en vitesse, en position et en trajectoire
* La lecture des données des capteurs
* Le contrôle des servomoteurs
* Le contrôle des moteurs pas à pas
* L'affichage du score
* Le contrôle de l'éclairage
* Le contrôle du fumigène

### Chaine de compilation
Il s'agit d'un projet Arduino classique, compilé pour la plateforme `Teensy 3.5`

* Installer l'IDE Arduino (v1.8.8) : https://www.arduino.cc/en/Main/Software
* Installer Teensyduino (v1.45) : https://www.pjrc.com/teensy/td_download.html
* Installer les bibliothèques nécessaires (liste complète ci-dessous)

> Type de carte: "Teensy 3.5"  
> USB Type: "Serial"  
> CPU Speed: "120MHz"  
> Optimize: "Faster"  

### Dépendances
#### Bibliothèques standard
Bibliothèques directement incluses avec l'IDE Arduino et Teensyduino, ou bien pouvant se télécharger via le gestionnaire de bibliothèques de l'IDE Arduino.
>* Ethernet (v2.0.0) : https://github.com/PaulStoffregen/Ethernet
>* Adafruit NeoPixel (v1.1.3) : https://github.com/adafruit/Adafruit_NeoPixel
>* Adafruit LED Backpack Library (v1.1.6) : https://github.com/adafruit/Adafruit-LED-Backpack-Library
>* StepperDriver (v1.1.4) : https://github.com/laurb9/StepperDriver

#### Bibliothèques tierces
Bibliothèques à installer manuellement (coiper le dossier de la librairie directement dans le répertoire `Arduino/librairies/`).
>* dynamixel_teensy (v1.0) : https://github.com/sylvaing19/dynamixel_teensy
>* ToF Sensor (v1.0) : -



## Code haut niveau (`high_level/`)

Programme en charge des tâches dites "haut niveau" qui nécessitent une puissance de calcul plus importante:

* Recherche de trajectoire
* Traitement haut niveau des capteurs
* Mémorisation de l'état du robot, des éléments de jeu et des obstacles vus
* Scripts, enchaînements d'actions écrits à l'avance


### Dépendances

Les dépendances du haut niveau ont toutes été développées en interne.

Injection de dépendances : https://github.com/PFGimenez/dependency-injector

Gestion de configuration : https://github.com/PFGimenez/config

Log : https://github.com/PFGimenez/log

Outils graphiques : https://github.com/PFGimenez/graphic-toolbox

Kraken, recherche de chemin courbe : https://github.com/PFGimenez/The-Kraken-Pathfinding

### Chaîne de compilation

Il s'agit d'un projet maven Java. Un JDK ainsi que maven sont nécessaires. Pour compiler le haut niveau, il suffit de se placer dans le répertoire `high_level` :

    cd high_level

Et de compiler grâce à maven:

    mvn clean compile assembly:single

Le fichier généré est `chariot-hl.jar` placé dans `high_level/target`. Des scripts placés dans `high_level` permet de l'utiliser facilement.

## Balise pour lire la girouette (`compass_reader/`)

Programme en charge de donner la valeur de la girouette. Ce programme tournera sur une raspberrypi zero équipée d’une webcam.

La girouette est elle balisée d’un tag ArUco 4x4 numéro 17 de 6cm de largeur, c’est ce tag qui sera lu pour indiquer la position de la girouette.

Pour augmenter la fiabilité, nous filmons en continue la girouette et nous basons sur les 5 dernières lectures
Si celles-ci sont toutes identiques, la valeur sera transmise au robot principale, sinon la balise renverra `???` en attente de plus d’information.

### Lancer le projet

* Installer python 3.7
* Installer les dépendances (`pip3 install -r requirements.txt`)
* Lancer le serveur web (`python main.py`)

### Comment tester ?

Une fois le server lancé, montrez l’ArUco ci-dessous à la webcam:

![ArUco17](./compass_reader/aruco-17.svg)

Allez sur [http://localhost:8080/](http://localhost:8080/) pour voir le résultat. Le point blanc vers le haut indiquera `North`, en bas `South` (et `???` si la valeur n’est pas stable)

Vous pouvez aussi voir les lectures dans la console si `DEBUG = True` dans `compass.py`

Pro tips : Si tu décides d’afficher l’ArUco sur ton téléphone, pense à désactiver la rotation automatique de l’écran 😅